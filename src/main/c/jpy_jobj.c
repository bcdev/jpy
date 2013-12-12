#include "jpy_module.h"
#include "jpy_jtype.h"
#include "jpy_jobj.h"
#include "jpy_jmethod.h"

JPy_JObj* JObj_FromType(JNIEnv* jenv, JPy_JType* type, jobject objectRef)
{
    JPy_JObj* obj;
    obj = (JPy_JObj*) PyObject_New(JPy_JObj, (PyTypeObject*) type);
    objectRef = (*jenv)->NewGlobalRef(jenv, objectRef);
    if (objectRef == NULL) {
        PyErr_NoMemory();
        return NULL;
    }
    obj->objectRef = objectRef;
    return obj;
}

/**
 * The JObj type's tp_init slot. Called when the type is used to create new instances (constructor).
 */
int JObj_init(JPy_JObj* self, PyObject* args, PyObject* kwds)
{
    JNIEnv* jenv;
    PyTypeObject* type;
    JPy_JType* jType;
    PyObject* constructor;
    JPy_JMethod* jMethod;
    jobject objectRef;
    jvalue* jArgs;
    JPy_ArgDisposer* jDisposers;

    JPY_GET_JENV(jenv, -1)

    type = ((PyObject*) self)->ob_type;

    constructor = PyDict_GetItemString(type->tp_dict, JPy_JINIT_ATTR_NAME);
    if (constructor == NULL) {
        PyErr_SetString(PyExc_RuntimeError, "no constructor found (missing JType attribute '" JPy_JINIT_ATTR_NAME "')");
        return -1;
    }

    if (!PyObject_TypeCheck(constructor, &JOverloadedMethod_Type)) {
        PyErr_SetString(PyExc_RuntimeError, "invalid JType attribute '"  JPy_JINIT_ATTR_NAME  "': expected type JOverloadedMethod_Type");
        return -1;
    }

    jType = (JPy_JType*) type;
    if (jType->classRef == NULL) {
        PyErr_SetString(PyExc_RuntimeError, "internal error: Java class reference is NULL");
        return -1;
    }

    jMethod = JOverloadedMethod_FindMethod((JPy_JOverloadedMethod*) constructor, args);
    if (jMethod == NULL) {
        return -1;
    }

    //printf("JObj_init 1\n");
    if (JMethod_CreateJArgs(jMethod, args, &jArgs, &jDisposers) < 0) {
        return -1;
    }

    //printf("JObj_init 2 jType->classRef=%p\n", jType->classRef);
    objectRef = (*jenv)->NewObjectA(jenv, jType->classRef, jMethod->mid, jArgs);
    if (objectRef == NULL) {
        PyErr_SetString(PyExc_RuntimeError, "Java constructor returned null");
        return -1;
    }

    if (jMethod->paramCount > 0) {
        JMethod_DisposeJArgs(jMethod->paramCount, jArgs, jDisposers);
    }

    // todo: add exception


    //printf("JObj_init 3\n");
    objectRef = (*jenv)->NewGlobalRef(jenv, objectRef);
    if (objectRef == NULL) {
        PyErr_NoMemory();
        return -1;
    }

    // Dont't forget that __init__ may be called multiple times
    if (self->objectRef != NULL) {
        (*jenv)->DeleteGlobalRef(jenv, self->objectRef);
    }

    self->objectRef = objectRef;

    //printf("JObj_init 4\n");
    return 0;
}

/**
 * The JObj type's tp_dealloc slot. Called when the reference count reaches zero.
 */
void JObj_dealloc(JPy_JObj* self)
{
    JNIEnv* jenv;

    if (JPy_IsDebug()) printf("JObj_dealloc: self->objectRef=%p\n", self->objectRef);

    jenv = JPy_GetJNIEnv();
    if (jenv != NULL) {
        if (self->objectRef != NULL) {
            (*jenv)->DeleteGlobalRef(jenv, self->objectRef);
        }
    }

    Py_TYPE(self)->tp_free((PyObject*) self);
}

int JObj_CompareTo(JPy_JObj* obj1, JPy_JObj* obj2)
{
    JNIEnv* jenv;
    jobject ref1;
    jobject ref2;
    int value;

    JPY_GET_JENV(jenv, -2)

    ref1 = obj1->objectRef;
    ref2 = obj2->objectRef;

    if (ref1 == ref2 || (*jenv)->IsSameObject(jenv, ref1, ref2)) {
        return 0;
    } else if ((*jenv)->IsInstanceOf(jenv, ref1, JPy_Comparable_JClass)) {
        // todo: optimize following code
        jclass classRef = (*jenv)->GetObjectClass(jenv, ref1);
        jmethodID mid = (*jenv)->GetMethodID(jenv, classRef, "compareTo", "(Ljava/lang/Object;)I");
        value = (*jenv)->CallIntMethod(jenv, ref1, mid, ref2);
    } else {
        value = (char*) ref1 - (char*) ref2;
    }

    return (value == 0) ? 0 : (value < 0) ? -1 : +1;
}

int JObj_Equals(JPy_JObj* obj1, JPy_JObj* obj2)
{
    JNIEnv* jenv;
    jobject ref1;
    jobject ref2;

    JPY_GET_JENV(jenv, -2)

    ref1 = obj1->objectRef;
    ref2 = obj2->objectRef;

    if ((*jenv)->IsSameObject(jenv, ref1, ref2)) {
        return 1;
    } else {
        return (*jenv)->CallIntMethod(jenv, ref1, JPy_Object_Equals_MID, ref2);
    }
}

/**
 * The JObj type's tp_richcompare slot. Python: obj1 <opid> obj2
 */
PyObject* JObj_richcompare(PyObject* obj1, PyObject* obj2, int opid)
{
    if (!JObj_Check(obj1) || !JObj_Check(obj2)) {
        Py_RETURN_FALSE;
    }
    if (opid == Py_LT) {
        int value = JObj_CompareTo((JPy_JObj*) obj1, (JPy_JObj*) obj2);
        if (value == -2) {
            return NULL;
        } else if (value == -1) {
            Py_RETURN_TRUE;
        } else {
            Py_RETURN_FALSE;
        }
    } else if (opid == Py_LE) {
        int value = JObj_CompareTo((JPy_JObj*) obj1, (JPy_JObj*) obj2);
        if (value == -2) {
            return NULL;
        } else if (value == -1 || value == 0) {
            Py_RETURN_TRUE;
        } else {
            Py_RETURN_FALSE;
        }
    } else if (opid == Py_GT) {
        int value = JObj_CompareTo((JPy_JObj*) obj1, (JPy_JObj*) obj2);
        if (value == -2) {
            return NULL;
        } else if (value == +1) {
            Py_RETURN_TRUE;
        } else {
            Py_RETURN_FALSE;
        }
    } else if (opid == Py_GE) {
        int value = JObj_CompareTo((JPy_JObj*) obj1, (JPy_JObj*) obj2);
        if (value == -2) {
            return NULL;
        } else if (value == +1 || value == 0) {
            Py_RETURN_TRUE;
        } else {
            Py_RETURN_FALSE;
        }
    } else if (opid == Py_EQ) {
        int value = JObj_Equals((JPy_JObj*) obj1, (JPy_JObj*) obj2);
        if (value == -2) {
            return NULL;
        } else if (value) {
            Py_RETURN_TRUE;
        } else {
            Py_RETURN_FALSE;
        }
    } else if (opid == Py_NE) {
        int value = JObj_Equals((JPy_JObj*) obj1, (JPy_JObj*) obj2);
        if (value == -2) {
            return NULL;
        } else if (value) {
            Py_RETURN_FALSE;
        } else {
            Py_RETURN_TRUE;
        }
    } else {
        PyErr_SetString(PyExc_RuntimeError, "internal error: unrecognized opid");
        return NULL;
    }
}

/**
 * The JObj type's tp_hash slot. Python: hash(obj)
 */
long JObj_hash(JPy_JObj* self)
{
    JNIEnv* jenv;
    jenv = JPy_GetJNIEnv();
    if (jenv != NULL) {
        return (*jenv)->CallIntMethod(jenv, self->objectRef, JPy_Object_HashCode_MID);
    }
    return -1;
}


/**
 * The JObj type's tp_repr slot. Python: repr(obj))
 */
PyObject* JObj_repr(JPy_JObj* self)
{
    return PyUnicode_FromFormat("%s(objectRef=%p)", Py_TYPE(self)->tp_name, self->objectRef);
}

/**
 * The JObj type's tp_str slot. Python: str(obj)
 */
PyObject* JObj_str(JPy_JObj* self)
{
    JNIEnv* jenv;
    JPY_GET_JENV(jenv, NULL)
    return JPy_ConvertJavaToStringToPythonString(jenv, self->objectRef);
}

/**
 * The JObj type's tp_getattro slot.
 * This is important: wrap callable objects of type JOverloadedMethod_Type into python methods so that
 * a method call to an instance x of class X becomes: x.m() --> X.m(x)
 */
PyObject* JObj_getattro(PyObject* self, PyObject* name)
{
    PyObject* value;

    // todo: implement a sepcial lookup: we need to override __getattro__ of JType (--> JType_getattro) as well so that we know if a method
    // is called on a class rather than on an instance. Using PyObject_GenericGetAttr will also call  JType_getattro,
    // but then we loose the information that a method is called on an instance and not on a class.
    value = PyObject_GenericGetAttr(self, name);
    if (value == NULL) {
        return NULL;
    }
    if (PyObject_TypeCheck(value, &JOverloadedMethod_Type)) {
        JPy_JOverloadedMethod* overloadedMethod = (JPy_JOverloadedMethod*) value;
        //printf("JObj_getattro: wrapping JOverloadedMethod, overloadCount=%d\n", PyList_Size(overloadedMethod->methodList));
        return PyMethod_New(value, self);
    } else {
        // printf("JObj_getattro: passing through\n");
    }
    return value;
}

/**
 * The JObj type's sq_length field of the tp_as_sequence slot. Called if len(obj) is called.
 * Only used for array types (type->componentType != NULL).
 */
Py_ssize_t JObj_sq_length(JPy_JObj* self)
{
    JNIEnv* jenv;
    jsize length;
    JPY_GET_JENV(jenv, -1)
    length = (*jenv)->GetArrayLength(jenv, self->objectRef);
    //printf("JObj_sq_length: length=%d\n", length);
    return (Py_ssize_t) length;
}

/*
 * The JObj type's sq_item field of the tp_as_sequence slot. Called if 'item = obj[index]' is used.
 * Only used for array types (type->componentType != NULL).
 */
PyObject* JObj_sq_item(JPy_JObj* self, Py_ssize_t index)
{
    JNIEnv* jenv;
    JPy_JType* type;
    PyTypeObject* componentType;
    jsize length;

    JPY_GET_JENV(jenv, NULL)

    //printf("JObj_sq_item: index=%d\n", index);

    type = (JPy_JType*) Py_TYPE(self);
    componentType = (PyTypeObject*) type->componentType;
    if (componentType == NULL) {
        PyErr_SetString(PyExc_RuntimeError, "internal error: object is not an array");
        return NULL;
    }

    // This is annoying and slow in Python 3.3.2: We must have this check, in order to raise an PyExc_IndexError,
    // otherwise Python functions such as list(jarr) will not succeed.
    // Tis is really strange, because n = sq_length() will be called and subsequent sq_item(index=0 ... n) calls will be done.
    length = (*jenv)->GetArrayLength(jenv, self->objectRef);
    if (index < 0 || index >= length) {
        PyErr_SetString(PyExc_IndexError, "Java array index out of bounds");
        return NULL;
    }

    if (componentType == JPy_JBoolean) {
        jboolean item;
        (*jenv)->GetBooleanArrayRegion(jenv, self->objectRef, (jsize) index, 1, &item);
        if (item) { Py_RETURN_TRUE; } else { Py_RETURN_FALSE; }
    } else if (componentType == JPy_JByte) {
        jbyte item;
        (*jenv)->GetByteArrayRegion(jenv, self->objectRef, (jsize) index, 1, &item);
        return PyLong_FromLong(item);
    } else if (componentType == JPy_JChar) {
        jchar item;
        (*jenv)->GetCharArrayRegion(jenv, self->objectRef, (jsize) index, 1, &item);
        return Py_BuildValue("C", item);
    } else if (componentType == JPy_JShort) {
        jshort item;
        (*jenv)->GetShortArrayRegion(jenv, self->objectRef, (jsize) index, 1, &item);
        return PyLong_FromLong(item);
    } else if (componentType == JPy_JInt) {
        jint item;
        (*jenv)->GetIntArrayRegion(jenv, self->objectRef, (jsize) index, 1, &item);
        return PyLong_FromLong(item);
    } else if (componentType == JPy_JLong) {
        jlong item;
        (*jenv)->GetLongArrayRegion(jenv, self->objectRef, (jsize) index, 1, &item);
        return PyLong_FromLongLong(item);
    } else if (componentType == JPy_JFloat) {
        jfloat item;
        (*jenv)->GetFloatArrayRegion(jenv, self->objectRef, (jsize) index, 1, &item);
        return PyFloat_FromDouble(item);
    } else if (componentType == JPy_JDouble) {
        jdouble item;
        (*jenv)->GetDoubleArrayRegion(jenv, self->objectRef, (jsize) index, 1, &item);
        return PyFloat_FromDouble(item);
    } else {
        jobject objectRef = (*jenv)->GetObjectArrayElement(jenv, self->objectRef, (jsize) index);
        if ((*jenv)->ExceptionCheck(jenv)) {
            (*jenv)->ExceptionDescribe(jenv);
            (*jenv)->ExceptionClear(jenv);
            PyErr_SetString(PyExc_RuntimeError, "index error");
            return NULL;
        }
        return JType_ConvertJavaToPythonObject(jenv, type->componentType, objectRef);
    }
}

/*
 * The JObj type's sq_ass_item field of the tp_as_sequence slot. Called if 'obj[index] = item' is used.
 * Only used for array types (type->componentType != NULL).
 */
int JObj_sq_ass_item(JPy_JObj* self, Py_ssize_t index, PyObject* pyItem)
{
    JNIEnv* jenv;
    JPy_JType* type;
    PyTypeObject* componentType;
    jobject elementRef;

    JPY_GET_JENV(jenv, -1)

    type = (JPy_JType*) Py_TYPE(self);
    componentType = (PyTypeObject*) type->componentType;
    if (type->componentType == NULL) {
        PyErr_SetString(PyExc_RuntimeError, "internal error: object is not an array");
        return -1;
    }

    // todo - the following item assignments are not value range checked
    if (componentType == JPy_JBoolean) {
        jboolean item = (jboolean)(pyItem == Py_True ? 1 : (pyItem == Py_False || pyItem == Py_None) ? 0 : PyLong_AsLong(pyItem) != 0);
        (*jenv)->SetBooleanArrayRegion(jenv, self->objectRef, (jsize) index, 1, &item);
    } else if (componentType == JPy_JByte) {
        jbyte item = (jbyte)(pyItem == Py_None ? 0 : PyLong_AsLong(pyItem));
        (*jenv)->SetByteArrayRegion(jenv, self->objectRef, (jsize) index, 1, &item);
    } else if (componentType == JPy_JChar) {
        jchar item = (jchar)(pyItem == Py_None ? 0 : PyLong_AsLong(pyItem));
        (*jenv)->SetCharArrayRegion(jenv, self->objectRef, (jsize) index, 1, &item);
    } else if (componentType == JPy_JShort) {
        jshort item = (jshort)(pyItem == Py_None ? 0 : PyLong_AsLong(pyItem));
        (*jenv)->SetShortArrayRegion(jenv, self->objectRef, (jsize) index, 1, &item);
    } else if (componentType == JPy_JInt) {
        jint item = (jint)(pyItem == Py_None ? 0 : PyLong_AsLong(pyItem));
        (*jenv)->SetIntArrayRegion(jenv, self->objectRef, (jsize) index, 1, &item);
    } else if (componentType == JPy_JLong) {
        jlong item = (jlong)(pyItem == Py_None ? 0 : PyLong_AsLongLong(pyItem));
        (*jenv)->SetLongArrayRegion(jenv, self->objectRef, (jsize) index, 1, &item);
    } else if (componentType == JPy_JFloat) {
        jfloat item = (jfloat)(pyItem == Py_None ? 0 : PyFloat_AsDouble(pyItem));
        (*jenv)->SetFloatArrayRegion(jenv, self->objectRef, (jsize) index, 1, &item);
    } else if (componentType == JPy_JDouble) {
        jdouble item = (jdouble)(pyItem == Py_None ? 0 : PyFloat_AsDouble(pyItem));
        (*jenv)->SetDoubleArrayRegion(jenv, self->objectRef, (jsize) index, 1, &item);
    } else {
        if (pyItem != Py_None) {
            if (JType_ConvertPythonToJavaObject(jenv, type->componentType, pyItem, &elementRef) < 0) {
                return -1;
            }
        } else {
            elementRef = NULL;
        }
        (*jenv)->SetObjectArrayElement(jenv, self->objectRef, (jsize) index, elementRef);
        if ((*jenv)->ExceptionCheck(jenv)) {
            (*jenv)->ExceptionDescribe(jenv);
            (*jenv)->ExceptionClear(jenv);
            PyErr_SetString(PyExc_RuntimeError, "index error");
            return -1;
        }
    }
    return 0;
}

/**
 * The JObj type's tp_as_sequence slot.
 * Implements the <sequence> interface for array types (type->componentType != NULL).
 */
static PySequenceMethods JObj_as_sequence = {
    (lenfunc) JObj_sq_length,            /* sq_length */
    NULL,   /* sq_concat */
    NULL,   /* sq_repeat */
    (ssizeargfunc) JObj_sq_item,         /* sq_item */
    NULL,   /* was_sq_slice */
    (ssizeobjargproc) JObj_sq_ass_item,  /* sq_ass_item */
    NULL,   /* was_sq_ass_slice */
    NULL,   /* sq_contains */
    NULL,   /* sq_inplace_concat */
    NULL,   /* sq_inplace_repeat */
};


int JType_InitSlots(JPy_JType* type)
{
    PyTypeObject* typeObj;

    typeObj = (PyTypeObject*) type;

    Py_REFCNT(typeObj) = 1;
    Py_TYPE(typeObj) = NULL;
    Py_SIZE(typeObj) = 0;
    // todo: The following lines are actually correct, but setting Py_TYPE(type) = &JType_Type results in an interpreter crash. Why?
    // This is still a problem because all the JType slots are actually never called (especially JType_getattro is needed to resolve unresolved JTypes)
    //Py_TYPE(type) = &JType_Type;
    //Py_SIZE(type) = sizeof (JPy_JType);
    //Py_INCREF(&JType_Type);

    typeObj->tp_basicsize = sizeof (JPy_JObj);
    typeObj->tp_itemsize = 0;
    typeObj->tp_base = (PyTypeObject*) type->superType;
    typeObj->tp_flags |= Py_TPFLAGS_DEFAULT;
    // If I uncomment the following line, I get an interpreter crash
    // (see also http://stackoverflow.com/questions/8066438/how-to-dynamically-create-a-derived-type-in-the-python-c-api)
    // typeObj->tp_flags |= Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HEAPTYPE;

    typeObj->tp_getattro = JObj_getattro;

    // todo: add  <sequence> protocol to 'java.lang.String' type.
    // Note that we have to do this after assigning the type to the global variable 'JPy_JString'.
    // However, this function (JType_InitSlots) is called *while* assigning the value 'JPy_JString'
    // so we cannot use to test against it here.

    // If this type is an array type, add support for the <sequence> protocol
    if (type->componentType != NULL) {
        typeObj->tp_as_sequence = &JObj_as_sequence;
    } else {
        typeObj->tp_as_sequence = NULL;
    }

    // todo: check Java Object type and create python protocols: Map/dict, List/list, Set/set

    typeObj->tp_alloc = PyType_GenericAlloc;
    typeObj->tp_new = PyType_GenericNew;
    typeObj->tp_init = (initproc) JObj_init;
    typeObj->tp_richcompare = (richcmpfunc) JObj_richcompare;
    typeObj->tp_hash = (hashfunc) JObj_hash;
    typeObj->tp_repr = (reprfunc) JObj_repr;
    typeObj->tp_str = (reprfunc) JObj_str;
    typeObj->tp_dealloc = (destructor) JObj_dealloc;

    // Check if we should set type.__module__ to the to the first part (up to the last dot) of the tp_name.
    // See http://docs.python.org/3/c-api/exceptions.html?highlight=pyerr_newexception#PyErr_NewException

    if (PyType_Ready(typeObj) < 0) {
        return -1;
    }

    return 0;
}

// This is only a good test as long JObj_init() is not used in other types
#define JPY_IS_JTYPE(T)  (((PyTypeObject*) T)->tp_init == (initproc) JObj_init)


int JObj_Check(PyObject* arg)
{
    return JPY_IS_JTYPE(Py_TYPE(arg));
}

int JType_Check(PyObject* arg)
{
    return PyType_Check(arg) && JPY_IS_JTYPE(arg);
}

