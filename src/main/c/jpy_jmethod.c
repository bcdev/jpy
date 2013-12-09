#include "jpy_module.h"
#include "jpy_jtype.h"
#include "jpy_jobj.h"
#include "jpy_jmethod.h"


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

int JMethod_AssessConversion(JPy_JMethod* jMethod, int argCount, PyObject* argTuple)
{
    JPy_ParamDescriptor* paramDescriptor;
    PyObject* arg;
    int matchValueSum;
    int matchValue;
    int i;
    int i0;

    if (jMethod->isStatic) {
        //printf("Static! jMethod->paramCount=%d, argCount=%d\n", jMethod->paramCount, argCount);
        if (jMethod->paramCount != argCount) {
            //printf("JMethod_AssessConversion 1\n");
            // argument count mismatch
            return 0;
        }
        i0 = 0;
    } else {
        PyObject* self;
        //printf("Non-Static! jMethod->paramCount=%d, argCount=%d\n", jMethod->paramCount, argCount);
        if (jMethod->paramCount != argCount - 1) {
            //printf("JMethod_AssessConversion 2\n");
            // argument count mismatch
            return 0;
        }
        self = PyTuple_GetItem(argTuple, 0);
        if (!JObj_Check(self)) {
            //printf("JMethod_AssessConversion 3\n");
            return 0;
        }
        i0 = 1;
    }

    if (jMethod->paramCount == 0) {
        //printf("JMethod_AssessConversion 4\n");
        // There will be no other method overloads with no parameters
        return 100;
    }

    paramDescriptor = jMethod->paramDescriptors;
    matchValueSum = 0;
    for (i = i0; i < argCount; i++) {

        arg = PyTuple_GetItem(argTuple, i);
        matchValue = paramDescriptor->assessToJValue(paramDescriptor->type, arg);
        if (matchValue == 0) {
            //printf("JMethod_AssessConversion 6\n");
            // current arg does not match parameter type at all
            return 0;
        }

        matchValueSum += matchValue;
        paramDescriptor++;

        if (JPy_IsDebug()) printf("JMethod_AssessConversion: argTuple[%d]: v=%d, valueSum=%d\n", i, matchValue, matchValueSum);
    }

    //printf("JMethod_AssessConversion 7\n");
    return matchValueSum;
}

int JMethod_CreateJArgs(JPy_JMethod* jMethod, PyObject* argTuple, jvalue** jArgs)
{
    JPy_ParamDescriptor* paramDescriptor;
    PyObject* arg;
    jvalue* jValue;
    int i, i0, argCount;
    jvalue* jValues;

    *jArgs = NULL;

    if (jMethod->paramCount == 0) {
        return 0;
    }

    jValues = PyMem_New(jvalue, jMethod->paramCount);
    if (jValues == NULL) {
        PyErr_NoMemory();
        return -1;
    }

    argCount = PyTuple_Size(argTuple);

    i0 = argCount - jMethod->paramCount;
    if (!(i0 == 0 || i0 == 1)) {
        PyErr_SetString(PyExc_RuntimeError, "internal error");
        return -1;
    }

    paramDescriptor = jMethod->paramDescriptors;
    jValue = jValues;
    for (i = i0; i < argCount; i++) {
        arg = PyTuple_GetItem(argTuple, i);
        if (paramDescriptor->convertToJValue(paramDescriptor->type, arg, jValue) < 0) {
            return -1;
        }
        paramDescriptor++;
        jValue++;
    }

    *jArgs = jValues;
    return 0;
}

/**
 * Convert a Java String to Python String.
 */
PyObject* JPy_FromJString(JNIEnv* jenv, JPy_JType* type, jstring stringRef)
{
    PyObject* returnValue;
    jsize length;
    const jchar* chars;

    if (stringRef == NULL) {
        return Py_BuildValue("");
    }

    length = (*jenv)->GetStringLength(jenv, stringRef);
    if (length == 0) {
        return Py_BuildValue("s", "");
    }

    chars = (*jenv)->GetStringChars(jenv, stringRef, NULL);
    returnValue = PyUnicode_FromWideChar(chars, length);
    (*jenv)->ReleaseStringChars(jenv, stringRef, chars);
    return returnValue;
}

/**
 * Convert any Java Object to Python Object.
 */
PyObject* JPy_FromJObject(JNIEnv* jenv, JPy_JType* type, jobject objectRef)
{
    JPy_JObj* returnValue;

    if (objectRef == NULL) {
        return Py_BuildValue("");
    }

    // todo: special treatment of arrays!

    // We can either
    //   1) create a JObj instance for objectRef using given type (current impl.), or
    //   2) get the actual class of the return value and lookup its JType and create an JObj instance.
    // It shall be an operation mode option for the 'jpy' module.
    // Using (1) is much faster, but (2) is more accurate.

    // actualClassRef = (*jenv)->GetObjectClass(jenv, objectRef);
    // actualType = JType_GetType(actualClassRef, JNI_FALSE);

    returnValue = JObj_FromType(jenv, type, objectRef);
    return (PyObject*) returnValue;
}


/**
 * Invoke a method. We have already ensured that the Python arguments and expected Java parameters match.
 */
PyObject* JMethod_InvokeMethod(JPy_JMethod* jMethod, JPy_JType* type, PyObject* argTuple)
{
    JNIEnv* jenv;
    jvalue* jArgs;
    PyObject* returnValue;
    PyTypeObject* returnType;

    JPY_GET_JENV(jenv, NULL)

    if (jMethod->returnDescriptor->type->componentType != NULL) {
        PyErr_SetString(PyExc_RuntimeError, "array return types not handled yet (TODO)");
        return NULL;
    }

    //printf("JMethod_InvokeMethod 1: typeCode=%c\n", typeCode);
    if (JMethod_CreateJArgs(jMethod, argTuple, &jArgs) < 0) {
        return NULL;
    }

    //printf("JMethod_InvokeMethod 2: typeCode=%c\n", typeCode);

    returnType = (PyTypeObject*) jMethod->returnDescriptor->type;

    if (jMethod->isStatic) {
        jclass classRef = type->classRef;

        if (returnType == JPy_JVoid) {
            (*jenv)->CallStaticVoidMethodA(jenv, classRef, jMethod->mid, jArgs);
            returnValue = Py_BuildValue("");
        } else if (returnType == JPy_JBoolean) {
            jboolean v = (*jenv)->CallStaticBooleanMethodA(jenv, classRef, jMethod->mid, jArgs);
            returnValue = PyBool_FromLong(v);
        } else if (returnType == JPy_JChar) {
            jchar v = (*jenv)->CallStaticCharMethodA(jenv, classRef, jMethod->mid, jArgs);
            returnValue = Py_BuildValue("C", v);
        } else if (returnType == JPy_JByte) {
            jbyte v = (*jenv)->CallStaticByteMethodA(jenv, classRef, jMethod->mid, jArgs);
            returnValue = PyLong_FromLong(v);
        } else if (returnType == JPy_JShort) {
            jshort v = (*jenv)->CallStaticShortMethodA(jenv, classRef, jMethod->mid, jArgs);
            returnValue = PyLong_FromLong(v);
        } else if (returnType == JPy_JInt) {
            jint v = (*jenv)->CallStaticIntMethodA(jenv, classRef, jMethod->mid, jArgs);
            returnValue = PyLong_FromLong(v);
        } else if (returnType == JPy_JLong) {
            jlong v = (*jenv)->CallStaticIntMethodA(jenv, classRef, jMethod->mid, jArgs);
            returnValue = PyLong_FromLongLong(v);
        } else if (returnType == JPy_JFloat) {
            jfloat v = (*jenv)->CallStaticFloatMethodA(jenv, classRef, jMethod->mid, jArgs);
            returnValue = PyFloat_FromDouble(v);
        } else if (returnType == JPy_JDouble) {
            jdouble v = (*jenv)->CallStaticDoubleMethodA(jenv, classRef, jMethod->mid, jArgs);
            returnValue = PyFloat_FromDouble(v);
        } else if (returnType == JPy_JString) {
            jstring v = (*jenv)->CallStaticObjectMethodA(jenv, classRef, jMethod->mid, jArgs);
            returnValue = JPy_FromJString(jenv, (JPy_JType*) returnType, v);
        } else {
            jobject v = (*jenv)->CallStaticObjectMethodA(jenv, classRef, jMethod->mid, jArgs);
            returnValue = JPy_FromJObject(jenv, (JPy_JType*) returnType, v);
        }

    } else {
        jobject objectRef;
        PyObject* self;

        self = PyTuple_GetItem(argTuple, 0);
        // Note it is already ensured that self is a JPy_JObj*
        objectRef = ((JPy_JObj*) self)->objectRef;

        if (returnType == JPy_JVoid) {
            (*jenv)->CallVoidMethodA(jenv, objectRef, jMethod->mid, jArgs);
            returnValue = Py_BuildValue("");
        } else if (returnType == JPy_JBoolean) {
            jboolean v = (*jenv)->CallBooleanMethodA(jenv, objectRef, jMethod->mid, jArgs);
            returnValue = PyBool_FromLong(v);
        } else if (returnType == JPy_JChar) {
            jchar v = (*jenv)->CallCharMethodA(jenv, objectRef, jMethod->mid, jArgs);
            returnValue = Py_BuildValue("c", v);
        } else if (returnType == JPy_JByte) {
            jbyte v = (*jenv)->CallByteMethodA(jenv, objectRef, jMethod->mid, jArgs);
            returnValue = PyLong_FromLong(v);
        } else if (returnType == JPy_JShort) {
            jshort v = (*jenv)->CallShortMethodA(jenv, objectRef, jMethod->mid, jArgs);
            returnValue = PyLong_FromLong(v);
        } else if (returnType == JPy_JInt) {
            jint v = (*jenv)->CallIntMethodA(jenv, objectRef, jMethod->mid, jArgs);
            returnValue = PyLong_FromLong(v);
        } else if (returnType == JPy_JLong) {
            jlong v = (*jenv)->CallIntMethodA(jenv, objectRef, jMethod->mid, jArgs);
            returnValue = PyLong_FromLongLong(v);
        } else if (returnType == JPy_JFloat) {
            jfloat v = (*jenv)->CallFloatMethodA(jenv, objectRef, jMethod->mid, jArgs);
            returnValue = PyFloat_FromDouble(v);
        } else if (returnType == JPy_JDouble) {
            jdouble v = (*jenv)->CallDoubleMethodA(jenv, objectRef, jMethod->mid, jArgs);
            returnValue = PyFloat_FromDouble(v);
        } else if (returnType == JPy_JString) {
            jstring v = (*jenv)->CallObjectMethodA(jenv, objectRef, jMethod->mid, jArgs);
            returnValue = JPy_FromJString(jenv, (JPy_JType*) returnType, v);
        } else {
            jobject v = (*jenv)->CallObjectMethodA(jenv, objectRef, jMethod->mid, jArgs);
            returnValue = JPy_FromJObject(jenv, (JPy_JType*) returnType, v);
        }
    }

    PyMem_Del(jArgs);

    return returnValue;
}


PyObject* JMethod_repr(JPy_JMethod* self)
{
    const char* name = PyUnicode_AsUTF8(self->name);
    return PyUnicode_FromFormat("%s(name='%s', param_count=%d, mid=%p)",
                                ((PyObject*)self)->ob_type->tp_name,
                                name,
                                self->paramCount,
                                self->mid);
}

PyObject* JMethod_str(JPy_JMethod* self)
{
    Py_INCREF(self->name);
    return self->name;
}


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
    NULL,                         /* tp_methods */
    NULL,                         /* tp_members */
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


JPy_JMethod* JOverloadedMethod_FindMethod(JPy_JOverloadedMethod* overloadedMethod, PyObject* argTuple)
{
    int overloadCount;
    int argCount;
    int matchCount;
    int matchValue;
    int matchValueMax;
    JPy_JMethod* currMethod;
    JPy_JMethod* bestMethod;
    int i;

    overloadCount = PyList_Size(overloadedMethod->methodList);
    if (overloadCount < 0) {
        return NULL;
    }
    if (overloadCount == 0) {
        PyErr_SetString(PyExc_RuntimeError, "internal error");
        return NULL;
    }

    argCount = PyTuple_Size(argTuple);
    matchCount = 0;
    matchValueMax = -1;
    bestMethod = NULL;

    if (JPy_IsDebug()) printf("JOverloadedMethod_FindMethod: overloadCount=%d\n", overloadCount);

    for (i = 0; i < overloadCount; i++) {
        currMethod = (JPy_JMethod*) PyList_GetItem(overloadedMethod->methodList, i);
        matchValue = JMethod_AssessConversion(currMethod, argCount, argTuple);

        if (JPy_IsDebug()) printf("JOverloadedMethod_FindMethod: methodList[%d]: matchValue=%d\n", i, matchValue);

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

    if (matchCount > 1) {
        PyErr_SetString(PyExc_RuntimeError, "too many matching Java method overloads found");
        return NULL;
    }

    if (bestMethod == NULL) {
        PyErr_SetString(PyExc_RuntimeError, "none of the Java method overloads match given arguments");
        return NULL;
    }

    return bestMethod;
}

JPy_JOverloadedMethod* JOverloadedMethod_New(JPy_JType* type, PyObject* name, JPy_JMethod* method)
{
    PyTypeObject* methodType = &JOverloadedMethod_Type;
    JPy_JOverloadedMethod* overloadedMethod;

    overloadedMethod = (JPy_JOverloadedMethod*) methodType->tp_alloc(methodType, 0);
    overloadedMethod->type = type;
    overloadedMethod->name = name;
    overloadedMethod->methodList = Py_BuildValue("[O]", method);

    Py_INCREF((PyObject*) type);
    Py_INCREF((PyObject*) name);
    Py_INCREF((PyObject*) overloadedMethod);

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
    Py_DECREF((PyObject*) self->type);
    Py_DECREF((PyObject*) self->name);
    Py_DECREF((PyObject*) self->methodList);
    Py_TYPE(self)->tp_free((PyObject*) self);
}

/**
 * The 'JOverloadedMethod' type's tp_call slot. Makes instances of the type callable.
 */
PyObject* JOverloadedMethod_call(JPy_JOverloadedMethod* self, PyObject *args, PyObject *kw)
{
    JPy_JMethod* method;

    //printf("JOverloadedMethod_call 1: self=%p\n", self);

    method = JOverloadedMethod_FindMethod(self, args);
    if (method == NULL) {
        return NULL;
    }

    //printf("JOverloadedMethod_call 2: method=%p\n", method);

    return JMethod_InvokeMethod(method, self->type, args);
}

/**
 * The 'JOverloadedMethod' type's tp_repr slot.
 */
PyObject* JOverloadedMethod_repr(JPy_JOverloadedMethod* self)
{
    const char* name = PyUnicode_AsUTF8(self->name);
    int methodCount = PyList_Size(self->methodList);
    return PyUnicode_FromFormat("%s(name='%s', methodCount=%d)",
                                ((PyObject*)self)->ob_type->tp_name,
                                name,
                                methodCount);
}

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
    NULL,                         /* tp_members */
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
