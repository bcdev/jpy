/*
 * Copyright (C) 2014 Brockmann Consult GmbH
 *
 * This program is free software; you can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation; either version 3 of the License,
 * or (at your option) any later version. This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
 * PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program; if not, see
 * http://www.gnu.org/licenses/
 */

#include "jpy_module.h"
#include "jpy_diag.h"
#include "jpy_jtype.h"
#include "jpy_jobj.h"
#include "jpy_jmethod.h"
#include "jpy_conv.h"
#include "jpy_compat.h"


JPy_JMethod* JMethod_New(PyObject* name,
                         int paramCount,
                         JPy_ParamDescriptor* paramDescriptors,
                         JPy_ReturnDescriptor* returnDescriptor,
                         jboolean isStatic,
                         jmethodID mid)
{
    PyTypeObject* type = &JMethod_Type;
    JPy_JMethod* method;

    method = (JPy_JMethod*) type->tp_alloc(type, 0);
    method->name = name;
    method->paramCount = paramCount;
    method->paramDescriptors = paramDescriptors;
    method->returnDescriptor = returnDescriptor;
    method->isStatic = isStatic;
    method->mid = mid;

    Py_INCREF(method->name);

    return method;
}


/**
 * The JMethod type's tp_dealloc slot. 
 */
void JMethod_dealloc(JPy_JMethod* self)
{
    JNIEnv* jenv;

    Py_DECREF(self->name);

    jenv = JPy_GetJNIEnv();
    if (jenv != NULL) {
        int i;
        for (i = 0; i < self->paramCount; i++) {
            Py_DECREF((self->paramDescriptors + i)->type);
        }
        Py_DECREF((self->returnDescriptor + i)->type);
    }

    PyMem_Del(self->paramDescriptors);
    PyMem_Del(self->returnDescriptor);
    
    Py_TYPE(self)->tp_free((PyObject*) self);
}

void JMethod_Del(JPy_JMethod* method)
{
    JMethod_dealloc(method);
}

/**
 * Matches the give Python argument tuple against the Java method's formal parameters.
 * Returns the sum of the i-th argument against the i-th Java parameter.
 * The maximum match value returned is 100 * method->paramCount.
 */
int JMethod_MatchPyArgs(JNIEnv* jenv, JPy_JMethod* method, int argCount, PyObject* pyArgs)
{
    JPy_ParamDescriptor* paramDescriptor;
    PyObject* pyArg;
    int matchValueSum;
    int matchValue;
    int i;
    int i0;

    if (method->isStatic) {
        //printf("Static! method->paramCount=%d, argCount=%d\n", method->paramCount, argCount);
        if (method->paramCount != argCount) {
            JPy_DIAG_PRINT(JPy_DIAG_F_METH, "JMethod_MatchPyArgs: argument count mismatch (matchValue=0)\n");
            // argument count mismatch
            return 0;
        }
        i0 = 0;
    } else {
        PyObject* self;
        //printf("Non-Static! method->paramCount=%d, argCount=%d\n", method->paramCount, argCount);
        if (method->paramCount != argCount - 1) {
            JPy_DIAG_PRINT(JPy_DIAG_F_METH, "JMethod_MatchPyArgs: argument count mismatch (matchValue=0)\n");
            // argument count mismatch
            return 0;
        }
        self = PyTuple_GetItem(pyArgs, 0);
        if (!JObj_Check(self)) {
            JPy_DIAG_PRINT(JPy_DIAG_F_METH, "JMethod_MatchPyArgs: self argument is not a Java object (matchValue=0)\n");
            return 0;
        }
        i0 = 1;
    }

    if (method->paramCount == 0) {
        JPy_DIAG_PRINT(JPy_DIAG_F_METH, "JMethod_MatchPyArgs: no-argument method (matchValue=100)\n");
        // There can't be any other method overloads with no parameters
        return 100;
    }

    paramDescriptor = method->paramDescriptors;
    matchValueSum = 0;
    for (i = i0; i < argCount; i++) {

        pyArg = PyTuple_GetItem(pyArgs, i);
        matchValue = paramDescriptor->MatchPyArg(jenv, paramDescriptor, pyArg);

        JPy_DIAG_PRINT(JPy_DIAG_F_METH, "JMethod_MatchPyArgs: pyArgs[%d]: matchValue=%d\n", i, matchValue);

        if (matchValue == 0) {
            //printf("JMethod_MatchPyArgs 6\n");
            // current pyArg does not match parameter type at all
            return 0;
        }

        matchValueSum += matchValue;
        paramDescriptor++;
    }

    //printf("JMethod_MatchPyArgs 7\n");
    return matchValueSum;
}

#define JPy_SUPPORT_RETURN_PARAMETER 1

PyObject* JMethod_FromJObject(JNIEnv* jenv, JPy_JMethod* method, PyObject* pyArgs, jvalue* jArgs, int argOffset, JPy_JType* returnType, jobject jReturnValue)
{
    #ifdef JPy_SUPPORT_RETURN_PARAMETER
    if (method->returnDescriptor->paramIndex >= 0) {
        jint paramIndex = method->returnDescriptor->paramIndex;
        PyObject* pyReturnArg = PyTuple_GetItem(pyArgs, paramIndex + argOffset);
        jobject jArg = jArgs[paramIndex].l;
        //printf("JMethod_FromJObject: paramIndex=%d, jArg=%p, isNone=%d\n", paramIndex, jArg, pyReturnArg == Py_None);
        if ((JObj_Check(pyReturnArg) || PyObject_CheckBuffer(pyReturnArg))
            && (*jenv)->IsSameObject(jenv, jReturnValue, jArg)) {
             Py_INCREF(pyReturnArg);
             return pyReturnArg;
        }
    }
    #endif
    return JPy_FromJObjectWithType(jenv, jReturnValue, returnType);
}

/**
 * Invoke a method. We have already ensured that the Python arguments and expected Java parameters match.
 */
PyObject* JMethod_InvokeMethod(JNIEnv* jenv, JPy_JMethod* method, JPy_JType* type, PyObject* pyArgs)
{
    jvalue* jArgs;
    JPy_ArgDisposer* argDisposers;
    PyObject* returnValue;
    JPy_JType* returnType;

    //printf("JMethod_InvokeMethod 1: typeCode=%c\n", typeCode);
    if (JMethod_CreateJArgs(jenv, method, pyArgs, &jArgs, &argDisposers) < 0) {
        return NULL;
    }

    //printf("JMethod_InvokeMethod 2: typeCode=%c\n", typeCode);

    returnType = method->returnDescriptor->type;
    returnValue = NULL;

    if (method->isStatic) {
        jclass classRef = type->classRef;

        JPy_DIAG_PRINT(JPy_DIAG_F_EXEC, "JMethod_InvokeMethod: calling static Java method %s#%s\n", type->javaName, JPy_AS_UTF8(method->name));

        if (returnType == JPy_JVoid) {
            (*jenv)->CallStaticVoidMethodA(jenv, classRef, method->mid, jArgs);
            JPy_ON_JAVA_EXCEPTION_GOTO(error);
            returnValue = JPy_FROM_JVOID();
        } else if (returnType == JPy_JBoolean) {
            jboolean v = (*jenv)->CallStaticBooleanMethodA(jenv, classRef, method->mid, jArgs);
            JPy_ON_JAVA_EXCEPTION_GOTO(error);
            returnValue = JPy_FROM_JBOOLEAN(v);
        } else if (returnType == JPy_JChar) {
            jchar v = (*jenv)->CallStaticCharMethodA(jenv, classRef, method->mid, jArgs);
            JPy_ON_JAVA_EXCEPTION_GOTO(error);
            returnValue = JPy_FROM_JCHAR(v);
        } else if (returnType == JPy_JByte) {
            jbyte v = (*jenv)->CallStaticByteMethodA(jenv, classRef, method->mid, jArgs);
            JPy_ON_JAVA_EXCEPTION_GOTO(error);
            returnValue = JPy_FROM_JBYTE(v);
        } else if (returnType == JPy_JShort) {
            jshort v = (*jenv)->CallStaticShortMethodA(jenv, classRef, method->mid, jArgs);
            JPy_ON_JAVA_EXCEPTION_GOTO(error);
            returnValue = JPy_FROM_JSHORT(v);
        } else if (returnType == JPy_JInt) {
            jint v = (*jenv)->CallStaticIntMethodA(jenv, classRef, method->mid, jArgs);
            JPy_ON_JAVA_EXCEPTION_GOTO(error);
            returnValue = JPy_FROM_JINT(v);
        } else if (returnType == JPy_JLong) {
            jlong v = (*jenv)->CallStaticIntMethodA(jenv, classRef, method->mid, jArgs);
            JPy_ON_JAVA_EXCEPTION_GOTO(error);
            returnValue = JPy_FROM_JLONG(v);
        } else if (returnType == JPy_JFloat) {
            jfloat v = (*jenv)->CallStaticFloatMethodA(jenv, classRef, method->mid, jArgs);
            JPy_ON_JAVA_EXCEPTION_GOTO(error);
            returnValue = JPy_FROM_JFLOAT(v);
        } else if (returnType == JPy_JDouble) {
            jdouble v = (*jenv)->CallStaticDoubleMethodA(jenv, classRef, method->mid, jArgs);
            JPy_ON_JAVA_EXCEPTION_GOTO(error);
            returnValue = JPy_FROM_JDOUBLE(v);
        } else if (returnType == JPy_JString) {
            jstring v = (*jenv)->CallStaticObjectMethodA(jenv, classRef, method->mid, jArgs);
            JPy_ON_JAVA_EXCEPTION_GOTO(error);
            returnValue = JPy_FromJString(jenv, v);
            (*jenv)->DeleteLocalRef(jenv, v);
        } else {
            jobject v = (*jenv)->CallStaticObjectMethodA(jenv, classRef, method->mid, jArgs);
            JPy_ON_JAVA_EXCEPTION_GOTO(error);
            returnValue = JMethod_FromJObject(jenv, method, pyArgs, jArgs, 0, returnType, v);
            (*jenv)->DeleteLocalRef(jenv, v);
        }

    } else {
        jobject objectRef;
        PyObject* self;

        JPy_DIAG_PRINT(JPy_DIAG_F_EXEC, "JMethod_InvokeMethod: calling Java method %s#%s\n", type->javaName, JPy_AS_UTF8(method->name));

        self = PyTuple_GetItem(pyArgs, 0);
        // Note it is already ensured that self is a JPy_JObj*
        objectRef = ((JPy_JObj*) self)->objectRef;

        if (returnType == JPy_JVoid) {
            (*jenv)->CallVoidMethodA(jenv, objectRef, method->mid, jArgs);
            JPy_ON_JAVA_EXCEPTION_GOTO(error);
            returnValue = JPy_FROM_JVOID();
        } else if (returnType == JPy_JBoolean) {
            jboolean v = (*jenv)->CallBooleanMethodA(jenv, objectRef, method->mid, jArgs);
            JPy_ON_JAVA_EXCEPTION_GOTO(error);
            returnValue = JPy_FROM_JBOOLEAN(v);
        } else if (returnType == JPy_JChar) {
            jchar v = (*jenv)->CallCharMethodA(jenv, objectRef, method->mid, jArgs);
            JPy_ON_JAVA_EXCEPTION_GOTO(error);
            returnValue = JPy_FROM_JCHAR(v);
        } else if (returnType == JPy_JByte) {
            jbyte v = (*jenv)->CallByteMethodA(jenv, objectRef, method->mid, jArgs);
            JPy_ON_JAVA_EXCEPTION_GOTO(error);
            returnValue = JPy_FROM_JBYTE(v);
        } else if (returnType == JPy_JShort) {
            jshort v = (*jenv)->CallShortMethodA(jenv, objectRef, method->mid, jArgs);
            JPy_ON_JAVA_EXCEPTION_GOTO(error);
            returnValue = JPy_FROM_JSHORT(v);
        } else if (returnType == JPy_JInt) {
            jint v = (*jenv)->CallIntMethodA(jenv, objectRef, method->mid, jArgs);
            JPy_ON_JAVA_EXCEPTION_GOTO(error);
            returnValue = JPy_FROM_JINT(v);
        } else if (returnType == JPy_JLong) {
            jlong v = (*jenv)->CallIntMethodA(jenv, objectRef, method->mid, jArgs);
            JPy_ON_JAVA_EXCEPTION_GOTO(error);
            returnValue = JPy_FROM_JLONG(v);
        } else if (returnType == JPy_JFloat) {
            jfloat v = (*jenv)->CallFloatMethodA(jenv, objectRef, method->mid, jArgs);
            JPy_ON_JAVA_EXCEPTION_GOTO(error);
            returnValue = JPy_FROM_JFLOAT(v);
        } else if (returnType == JPy_JDouble) {
            jdouble v = (*jenv)->CallDoubleMethodA(jenv, objectRef, method->mid, jArgs);
            JPy_ON_JAVA_EXCEPTION_GOTO(error);
            returnValue = JPy_FROM_JDOUBLE(v);
        } else if (returnType == JPy_JString) {
            jstring v = (*jenv)->CallObjectMethodA(jenv, objectRef, method->mid, jArgs);
            JPy_ON_JAVA_EXCEPTION_GOTO(error);
            returnValue = JPy_FromJString(jenv, v);
            (*jenv)->DeleteLocalRef(jenv, v);
        } else {
            jobject v = (*jenv)->CallObjectMethodA(jenv, objectRef, method->mid, jArgs);
            JPy_ON_JAVA_EXCEPTION_GOTO(error);
            returnValue = JMethod_FromJObject(jenv, method, pyArgs, jArgs, 1, returnType, v);
            (*jenv)->DeleteLocalRef(jenv, v);
        }
    }

error:
    if (jArgs != NULL) {
        JMethod_DisposeJArgs(jenv, method->paramCount, jArgs, argDisposers);
    }

    return returnValue;
}

int JMethod_CreateJArgs(JNIEnv* jenv, JPy_JMethod* method, PyObject* pyArgs, jvalue** argValuesRet, JPy_ArgDisposer** argDisposersRet)
{
    JPy_ParamDescriptor* paramDescriptor;
    int i, i0, argCount;
    PyObject* pyArg;
    jvalue* jValue;
    jvalue* jValues;
    JPy_ArgDisposer* argDisposer;
    JPy_ArgDisposer* argDisposers;

    if (method->paramCount == 0) {
        *argValuesRet = NULL;
        *argDisposersRet = NULL;
        return 0;
    }

    argCount = PyTuple_Size(pyArgs);

    i0 = argCount - method->paramCount;
    if (!(i0 == 0 || i0 == 1)) {
        PyErr_SetString(PyExc_RuntimeError, "internal error");
        return -1;
    }

    jValues = PyMem_New(jvalue, method->paramCount);
    if (jValues == NULL) {
        PyErr_NoMemory();
        return -1;
    }

    argDisposers = PyMem_New(JPy_ArgDisposer, method->paramCount);
    if (argDisposers == NULL) {
        PyMem_Del(jValues);
        PyErr_NoMemory();
        return -1;
    }

    paramDescriptor = method->paramDescriptors;
    jValue = jValues;
    argDisposer = argDisposers;
    for (i = i0; i < argCount; i++) {
        pyArg = PyTuple_GetItem(pyArgs, i);
        jValue->l = 0;
        argDisposer->data = NULL;
        argDisposer->DisposeArg = NULL;
        if (paramDescriptor->ConvertPyArg(jenv, paramDescriptor, pyArg, jValue, argDisposer) < 0) {
            PyMem_Del(jValues);
            PyMem_Del(argDisposers);
            return -1;
        }
        paramDescriptor++;
        jValue++;
        argDisposer++;
    }

    *argValuesRet = jValues;
    *argDisposersRet = argDisposers;
    return 0;
}

void JMethod_DisposeJArgs(JNIEnv* jenv, int paramCount, jvalue* jArgs, JPy_ArgDisposer* argDisposers)
{
    jvalue* jArg;
    JPy_ArgDisposer* argDisposer;
    int index;

    jArg = jArgs;
    argDisposer = argDisposers;

    for (index = 0; index < paramCount; index++) {
        if (argDisposer->DisposeArg != NULL) {
            argDisposer->DisposeArg(jenv, jArg, argDisposer->data);
        }
        jArg++;
        argDisposer++;
    }

    PyMem_Del(jArgs);
    PyMem_Del(argDisposers);
}


PyObject* JMethod_repr(JPy_JMethod* self)
{
    const char* name = JPy_AS_UTF8(self->name);
    return PyUnicode_FromFormat("%s(name='%s', param_count=%d, is_static=%d, mid=%p)",
                                ((PyObject*)self)->ob_type->tp_name,
                                name,
                                self->paramCount,
                                self->isStatic,
                                self->mid);
}

PyObject* JMethod_str(JPy_JMethod* self)
{
    Py_INCREF(self->name);
    return self->name;
}


static PyMemberDef JMethod_members[] =
{
    {"name",        T_OBJECT_EX, offsetof(JPy_JMethod, name),       READONLY, "Method name"},
    {"param_count", T_INT,       offsetof(JPy_JMethod, paramCount), READONLY, "Number of method parameters"},
    {"is_static",   T_BOOL,      offsetof(JPy_JMethod, isStatic),   READONLY, "Tests if this is a static method"},
    {NULL}  /* Sentinel */
};

#define JMethod_CHECK_PARAMETER_INDEX(self, index) \
    if (index < 0 || index >= self->paramCount) { \
        PyErr_SetString(PyExc_IndexError, "invalid parameter index"); \
        return NULL; \
    }


PyObject* JMethod_get_param_type(JPy_JMethod* self, PyObject* args)
{
    PyObject* type;
    int index;
    if (!PyArg_ParseTuple(args, "i:get_param_type", &index)) {
        return NULL;
    }
    JMethod_CHECK_PARAMETER_INDEX(self, index);
    type = (PyObject*) self->paramDescriptors[index].type;
    Py_INCREF(type);
    return type;
}

PyObject* JMethod_is_param_mutable(JPy_JMethod* self, PyObject* args)
{
    int index;
    int value;
    if (!PyArg_ParseTuple(args, "i:is_param_mutable", &index)) {
        return NULL;
    }
    JMethod_CHECK_PARAMETER_INDEX(self, index);
    value = self->paramDescriptors[index].isMutable;
    return PyBool_FromLong(value);
}

PyObject* JMethod_set_param_mutable(JPy_JMethod* self, PyObject* args)
{
    int index;
    int value;
    if (!PyArg_ParseTuple(args, "ip:set_param_mutable", &index, &value)) {
        return NULL;
    }
    JMethod_CHECK_PARAMETER_INDEX(self, index);
    self->paramDescriptors[index].isMutable = value;
    return Py_BuildValue("");
}

PyObject* JMethod_is_param_return(JPy_JMethod* self, PyObject* args)
{
    int index = 0;
    int value = 0;
    if (!PyArg_ParseTuple(args, "i:is_param_return", &index)) {
        return NULL;
    }
    JMethod_CHECK_PARAMETER_INDEX(self, index);
    value = self->paramDescriptors[index].isReturn;
    return PyBool_FromLong(value);
}

PyObject* JMethod_set_param_return(JPy_JMethod* self, PyObject* args)
{
    int index = 0;
    int value = 0;
    if (!PyArg_ParseTuple(args, "ip:set_param_return", &index, &value)) {
        return NULL;
    }
    JMethod_CHECK_PARAMETER_INDEX(self, index);
    self->paramDescriptors[index].isReturn = value;
    if (value) {
        self->returnDescriptor->paramIndex = index;
    }
    return Py_BuildValue("");
}


static PyMethodDef JMethod_methods[] =
{
    {"get_param_type",    (PyCFunction) JMethod_get_param_type,    METH_VARARGS, "Gets the type of the parameter given by index"},
    {"is_param_mutable",  (PyCFunction) JMethod_is_param_mutable,  METH_VARARGS, "Tests if the method parameter given by index is mutable"},
    {"is_param_return",   (PyCFunction) JMethod_is_param_return,   METH_VARARGS, "Tests if the method parameter given by index is the return value"},
    {"set_param_mutable", (PyCFunction) JMethod_set_param_mutable, METH_VARARGS, "Sets whether the method parameter given by index is mutable"},
    {"set_param_return",  (PyCFunction) JMethod_set_param_return,  METH_VARARGS, "Sets whether the method parameter given by index is the return value"},
    {NULL}  /* Sentinel */
};

/**
 * Implements the BeamPy_JObjectType class singleton.
 */
PyTypeObject JMethod_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "jpy.JMethod",                /* tp_name */
    sizeof (JPy_JMethod),         /* tp_basicsize */
    0,                            /* tp_itemsize */
    (destructor)JMethod_dealloc,  /* tp_dealloc */
    NULL,                         /* tp_print */
    NULL,                         /* tp_getattr */
    NULL,                         /* tp_setattr */
    NULL,                         /* tp_reserved */
    (reprfunc)JMethod_repr,       /* tp_repr */
    NULL,                         /* tp_as_number */
    NULL,                         /* tp_as_sequence */
    NULL,                         /* tp_as_mapping */
    NULL,                         /* tp_hash  */
    NULL,                         /* tp_call */
    (reprfunc)JMethod_str,        /* tp_str */
    NULL,                         /* tp_getattro */
    NULL,                         /* tp_setattro */
    NULL,                         /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,           /* tp_flags */
    "Java Method Wrapper",        /* tp_doc */
    NULL,                         /* tp_traverse */
    NULL,                         /* tp_clear */
    NULL,                         /* tp_richcompare */
    0,                            /* tp_weaklistoffset */
    NULL,                         /* tp_iter */
    NULL,                         /* tp_iternext */
    JMethod_methods,              /* tp_methods */
    JMethod_members,              /* tp_members */
    NULL,                         /* tp_getset */
    NULL,                         /* tp_base */
    NULL,                         /* tp_dict */
    NULL,                         /* tp_descr_get */
    NULL,                         /* tp_descr_set */
    0,                            /* tp_dictoffset */
    NULL,                         /* tp_init */
    NULL,                         /* tp_alloc */
    NULL,                         /* tp_new */
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  JOverloadedMethod

typedef struct JPy_MethodFindResult
{
    JPy_JMethod* method;
    int matchValue;
    int matchCount;
}
JPy_MethodFindResult;

JPy_JMethod* JOverloadedMethod_FindMethod0(JNIEnv* jenv, JPy_JOverloadedMethod* overloadedMethod, PyObject* pyArgs, JPy_MethodFindResult* result)
{
    int overloadCount;
    int argCount;
    int matchCount;
    int matchValue;
    int matchValueMax;
    JPy_JMethod* currMethod;
    JPy_JMethod* bestMethod;
    int i;

    result->method = NULL;
    result->matchValue = 0;
    result->matchCount = 0;

    overloadCount = PyList_Size(overloadedMethod->methodList);
    if (overloadCount <= 0) {
        PyErr_SetString(PyExc_RuntimeError, "internal error: invalid overloadedMethod->methodList");
        return NULL;
    }

    argCount = PyTuple_Size(pyArgs);
    matchCount = 0;
    matchValueMax = -1;
    bestMethod = NULL;

    JPy_DIAG_PRINT(JPy_DIAG_F_METH, "JOverloadedMethod_FindMethod0: method '%s#%s': overloadCount=%d\n",
                              overloadedMethod->declaringClass->javaName, JPy_AS_UTF8(overloadedMethod->name), overloadCount);

    for (i = 0; i < overloadCount; i++) {
        currMethod = (JPy_JMethod*) PyList_GetItem(overloadedMethod->methodList, i);
        matchValue = JMethod_MatchPyArgs(jenv, currMethod, argCount, pyArgs);

        JPy_DIAG_PRINT(JPy_DIAG_F_METH, "JOverloadedMethod_FindMethod0: methodList[%d]: paramCount=%d, matchValue=%d\n", i,
                                  currMethod->paramCount, matchValue);

        if (matchValue > 0) {
            if (matchValue > matchValueMax) {
                matchValueMax = matchValue;
                bestMethod = currMethod;
                matchCount = 1;
            } else if (matchValue == matchValueMax) {
                matchCount++;
            }
            if (matchValue >= 100 * currMethod->paramCount) {
                // we can't get any better (if so, we have an internal problem)
                break;
            }
        }
    }

    if (bestMethod == NULL) {
        matchValueMax = 0;
        matchCount = 0;
    }

    result->method = bestMethod;
    result->matchValue = matchValueMax;
    result->matchCount = matchCount;

    return bestMethod;
}

JPy_JMethod* JOverloadedMethod_FindMethod(JNIEnv* jenv, JPy_JOverloadedMethod* overloadedMethod, PyObject* pyArgs, jboolean visitSuperClass)
{
    JPy_JOverloadedMethod* currentOM;
    JPy_MethodFindResult result;
    JPy_MethodFindResult bestResult;
    JPy_JType* superClass;
    PyObject* superOM;

    if ((JPy_DiagFlags & JPy_DIAG_F_METH) != 0) {
        int i, argCount = PyTuple_Size(pyArgs);
        printf("JOverloadedMethod_FindMethod: argCount=%d, visitSuperClass=%d\n", argCount, visitSuperClass);
        for (i = 0; i < argCount; i++) {
            PyObject* pyArg = PyTuple_GetItem(pyArgs, i);
            printf("\tPy_TYPE(pyArgs[%d])->tp_name = %s\n", i, Py_TYPE(pyArg)->tp_name);
        }
    }

    bestResult.method = NULL;
    bestResult.matchValue = 0;
    bestResult.matchCount = 0;

    currentOM = overloadedMethod;
    while (1) {
        if (JOverloadedMethod_FindMethod0(jenv, currentOM, pyArgs, &result) < 0) {
            // oops, error
            return NULL;
        }
        if (result.method != NULL) {
            if (result.matchValue >= 100 * result.method->paramCount) {
                // We can't get any better.
                return result.method;
            } else if (result.matchValue > 0 && result.matchValue > bestResult.matchValue) {
                // We may have better matching methods overloads in the super class (if any)
                bestResult = result;
            }
        }

        if (visitSuperClass) {
            superClass = currentOM->declaringClass->superType;
            if (superClass != NULL) {
                superOM = JType_GetOverloadedMethod(jenv, superClass, currentOM->name, JNI_TRUE);
            } else {
                superOM = Py_None;
            }
        } else {
            superOM = Py_None;
        }

        if (superOM == NULL) {
            // oops, error
            return NULL;
        } else if (superOM == Py_None) {
            // no overloaded methods found in super class, so return best result found so far
            if (bestResult.method == NULL) {
                PyErr_SetString(PyExc_RuntimeError, "no matching Java method overloads found");
                return NULL;
            } else if (bestResult.matchCount > 1) {
                PyErr_SetString(PyExc_RuntimeError, "ambiguous Java method call, too many matching method overloads found");
                return NULL;
            } else {
                return bestResult.method;
            }
        } else {
            // Continue trying with overloads from super type
            currentOM = (JPy_JOverloadedMethod*) superOM;
        }
    }

    // Should never come here
    PyErr_SetString(PyExc_RuntimeError, "internal error");
    return NULL;
}

JPy_JOverloadedMethod* JOverloadedMethod_New(JPy_JType* declaringClass, PyObject* name, JPy_JMethod* method)
{
    PyTypeObject* methodType = &JOverloadedMethod_Type;
    JPy_JOverloadedMethod* overloadedMethod;

    overloadedMethod = (JPy_JOverloadedMethod*) methodType->tp_alloc(methodType, 0);
    overloadedMethod->declaringClass = declaringClass;
    overloadedMethod->name = name;
    overloadedMethod->methodList = PyList_New(0);

    Py_INCREF((PyObject*) overloadedMethod->declaringClass);
    Py_INCREF((PyObject*) overloadedMethod->name);
    Py_INCREF((PyObject*) overloadedMethod);

    JOverloadedMethod_AddMethod(overloadedMethod, method);

    return overloadedMethod;
}

int JOverloadedMethod_AddMethod(JPy_JOverloadedMethod* overloadedMethod, JPy_JMethod* method)
{
    return PyList_Append(overloadedMethod->methodList, (PyObject*) method);
}

/**
 * The 'JOverloadedMethod' type's tp_dealloc slot.
 */
void JOverloadedMethod_dealloc(JPy_JOverloadedMethod* self)
{
    Py_DECREF((PyObject*) self->declaringClass);
    Py_DECREF((PyObject*) self->name);
    Py_DECREF((PyObject*) self->methodList);
    Py_TYPE(self)->tp_free((PyObject*) self);
}

/**
 * The 'JOverloadedMethod' type's tp_call slot. Makes instances of the 'JOverloadedMethod' type callable.
 */
PyObject* JOverloadedMethod_call(JPy_JOverloadedMethod* self, PyObject *args, PyObject *kw)
{
    JNIEnv* jenv;
    JPy_JMethod* method;

    JPy_GET_JNI_ENV_OR_RETURN(jenv, NULL)

    method = JOverloadedMethod_FindMethod(jenv, self, args, JNI_TRUE);
    if (method == NULL) {
        return NULL;
    }

    return JMethod_InvokeMethod(jenv, method, self->declaringClass, args);
}

/**
 * The 'JOverloadedMethod' type's tp_repr slot.
 */
PyObject* JOverloadedMethod_repr(JPy_JOverloadedMethod* self)
{
    const char* name = JPy_AS_UTF8(self->name);
    int methodCount = PyList_Size(self->methodList);
    return PyUnicode_FromFormat("%s(name='%s', methodCount=%d)",
                                ((PyObject*)self)->ob_type->tp_name,
                                name,
                                methodCount);
}

static PyMemberDef JOverloadedMethod_members[] =
{
    {"name",    T_OBJECT_EX, offsetof(JPy_JOverloadedMethod, name),       READONLY, "Overloaded method name"},
    {"methods", T_OBJECT_EX, offsetof(JPy_JOverloadedMethod, methodList), READONLY, "List of methods"},
    {NULL}  /* Sentinel */
};

/**
 * The 'JOverloadedMethod' type's tp_str slot.
 */
PyObject* JOverloadedMethod_str(JPy_JOverloadedMethod* self)
{
    Py_INCREF(self->name);
    return self->name;
}

PyTypeObject JOverloadedMethod_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "jpy.JOverloadedMethod",                /* tp_name */
    sizeof (JPy_JOverloadedMethod),         /* tp_basicsize */
    0,                            /* tp_itemsize */
    (destructor)JOverloadedMethod_dealloc,  /* tp_dealloc */
    NULL,                         /* tp_print */
    NULL,                         /* tp_getattr */
    NULL,                         /* tp_setattr */
    NULL,                         /* tp_reserved */
    (reprfunc)JOverloadedMethod_repr,       /* tp_repr */
    NULL,                         /* tp_as_number */
    NULL,                         /* tp_as_sequence */
    NULL,                         /* tp_as_mapping */
    NULL,                         /* tp_hash  */
    (ternaryfunc)JOverloadedMethod_call,    /* tp_call */
    (reprfunc)JOverloadedMethod_str,        /* tp_str */
    NULL,                         /* tp_getattro */
    NULL,                         /* tp_setattro */
    NULL,                         /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,           /* tp_flags */
    "Java Overloaded Method",     /* tp_doc */
    NULL,                         /* tp_traverse */
    NULL,                         /* tp_clear */
    NULL,                         /* tp_richcompare */
    0,                            /* tp_weaklistoffset */
    NULL,                         /* tp_iter */
    NULL,                         /* tp_iternext */
    NULL,                         /* tp_methods */
    JOverloadedMethod_members,    /* tp_members */
    NULL,                         /* tp_getset */
    NULL,                         /* tp_base */
    NULL,                         /* tp_dict */
    NULL,                         /* tp_descr_get */
    NULL,                         /* tp_descr_set */
    0,                            /* tp_dictoffset */
    NULL,                         /* tp_init */
    NULL,                         /* tp_alloc */
    NULL,                         /* tp_new */
};
