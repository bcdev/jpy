#include "jpy_module.h"
#include "jpy_jtype.h"
#include "jpy_jobj.h"
#include "jpy_jfield.h"


JPy_JField* JField_New(JPy_JType* declaringClass, PyObject* fieldName, JPy_JType* fieldType, jboolean isStatic, jboolean isFinal, jfieldID fid)
{
    PyTypeObject* type = &JField_Type;
    JPy_JField* field;

    field = (JPy_JField*) type->tp_alloc(type, 0);
    field->declaringClass = declaringClass;
    field->name = fieldName;
    field->type = fieldType;
    field->isStatic = isStatic;
    field->isFinal = isFinal;
    field->fid = fid;

    Py_INCREF(field->name);
    Py_INCREF(field->type);

    return field;
}

/**
 * The JField type's tp_dealloc slot.
 */
void JField_dealloc(JPy_JField* self)
{
    Py_DECREF(self->name);
    Py_DECREF(self->type);
    Py_TYPE(self)->tp_free((PyObject*) self);
}

void JField_Del(JPy_JField* field)
{
    JField_dealloc(field);
}

/*
PyObject* JMethod_GetField(JNIEnv* jenv, JPy_JField* field)
{
    jvalue* jArgs;
    JPy_ArgDisposer* jDisposers;
    PyObject* returnValue;
    PyTypeObject* returnType;

    //printf("JMethod_InvokeMethod 1: typeCode=%c\n", typeCode);
    if (JMethod_CreateJArgs(jenv, method, argTuple, &jArgs, &jDisposers) < 0) {
        return NULL;
    }

    //printf("JMethod_InvokeMethod 2: typeCode=%c\n", typeCode);

    returnType = (PyTypeObject*) method->returnDescriptor->type;

    if (method->isStatic) {
        jclass classRef = type->classRef;

        if (returnType == JPy_JVoid) {
            (*jenv)->CallStaticVoidMethodA(jenv, classRef, method->mid, jArgs);
            returnValue = Py_BuildValue("");
        } else if (returnType == JPy_JBoolean) {
            jboolean v = (*jenv)->CallStaticBooleanMethodA(jenv, classRef, method->mid, jArgs);
            returnValue = PyBool_FromLong(v);
        } else if (returnType == JPy_JChar) {
            jchar v = (*jenv)->CallStaticCharMethodA(jenv, classRef, method->mid, jArgs);
            returnValue = Py_BuildValue("C", v);
        } else if (returnType == JPy_JByte) {
            jbyte v = (*jenv)->CallStaticByteMethodA(jenv, classRef, method->mid, jArgs);
            returnValue = PyLong_FromLong(v);
        } else if (returnType == JPy_JShort) {
            jshort v = (*jenv)->CallStaticShortMethodA(jenv, classRef, method->mid, jArgs);
            returnValue = PyLong_FromLong(v);
        } else if (returnType == JPy_JInt) {
            jint v = (*jenv)->CallStaticIntMethodA(jenv, classRef, method->mid, jArgs);
            returnValue = PyLong_FromLong(v);
        } else if (returnType == JPy_JLong) {
            jlong v = (*jenv)->CallStaticIntMethodA(jenv, classRef, method->mid, jArgs);
            returnValue = PyLong_FromLongLong(v);
        } else if (returnType == JPy_JFloat) {
            jfloat v = (*jenv)->CallStaticFloatMethodA(jenv, classRef, method->mid, jArgs);
            returnValue = PyFloat_FromDouble(v);
        } else if (returnType == JPy_JDouble) {
            jdouble v = (*jenv)->CallStaticDoubleMethodA(jenv, classRef, method->mid, jArgs);
            returnValue = PyFloat_FromDouble(v);
        } else if (returnType == JPy_JString) {
            jstring v = (*jenv)->CallStaticObjectMethodA(jenv, classRef, method->mid, jArgs);
            returnValue = JPy_FromJString(jenv, (JPy_JType*) returnType, v);
        } else {
            jobject v = (*jenv)->CallStaticObjectMethodA(jenv, classRef, method->mid, jArgs);
            returnValue = JPy_FromJObject(jenv, (JPy_JType*) returnType, v);
        }

    } else {
        jobject objectRef;
        PyObject* self;

        self = PyTuple_GetItem(argTuple, 0);
        // Note it is already ensured that self is a JPy_JObj*
        objectRef = ((JPy_JObj*) self)->objectRef;

        if (returnType == JPy_JVoid) {
            (*jenv)->CallVoidMethodA(jenv, objectRef, method->mid, jArgs);
            returnValue = Py_BuildValue("");
        } else if (returnType == JPy_JBoolean) {
            jboolean v = (*jenv)->CallBooleanMethodA(jenv, objectRef, method->mid, jArgs);
            returnValue = PyBool_FromLong(v);
        } else if (returnType == JPy_JChar) {
            jchar v = (*jenv)->CallCharMethodA(jenv, objectRef, method->mid, jArgs);
            returnValue = Py_BuildValue("c", v);
        } else if (returnType == JPy_JByte) {
            jbyte v = (*jenv)->CallByteMethodA(jenv, objectRef, method->mid, jArgs);
            returnValue = PyLong_FromLong(v);
        } else if (returnType == JPy_JShort) {
            jshort v = (*jenv)->CallShortMethodA(jenv, objectRef, method->mid, jArgs);
            returnValue = PyLong_FromLong(v);
        } else if (returnType == JPy_JInt) {
            jint v = (*jenv)->CallIntMethodA(jenv, objectRef, method->mid, jArgs);
            returnValue = PyLong_FromLong(v);
        } else if (returnType == JPy_JLong) {
            jlong v = (*jenv)->CallIntMethodA(jenv, objectRef, method->mid, jArgs);
            returnValue = PyLong_FromLongLong(v);
        } else if (returnType == JPy_JFloat) {
            jfloat v = (*jenv)->CallFloatMethodA(jenv, objectRef, method->mid, jArgs);
            returnValue = PyFloat_FromDouble(v);
        } else if (returnType == JPy_JDouble) {
            jdouble v = (*jenv)->CallDoubleMethodA(jenv, objectRef, method->mid, jArgs);
            returnValue = PyFloat_FromDouble(v);
        } else if (returnType == JPy_JString) {
            jstring v = (*jenv)->CallObjectMethodA(jenv, objectRef, method->mid, jArgs);
            returnValue = JPy_FromJString(jenv, (JPy_JType*) returnType, v);
        } else {
            jobject v = (*jenv)->CallObjectMethodA(jenv, objectRef, method->mid, jArgs);
            returnValue = JPy_FromJObject(jenv, (JPy_JType*) returnType, v);
        }
    }

    if (jArgs != NULL) {
        JMethod_DisposeJArgs(method->paramCount, jArgs, jDisposers);
    }

    return returnValue;
}
*/

PyObject* JField_repr(JPy_JField* self)
{
    const char* name = PyUnicode_AsUTF8(self->name);
    return PyUnicode_FromFormat("%s(name='%s', is_static=%d, is_final=%d, fid=%p)",
                                ((PyObject*)self)->ob_type->tp_name,
                                name,
                                self->isStatic,
                                self->isFinal,
                                self->fid);
}

PyObject* JField_str(JPy_JField* self)
{
    Py_INCREF(self->name);
    return self->name;
}


static PyMemberDef JField_members[] =
{
    {"name",        T_OBJECT_EX, offsetof(JPy_JField, name),       READONLY, "Field name"},
    {"is_static",   T_BOOL,      offsetof(JPy_JField, isStatic),   READONLY, "Tests if this is a static field"},
    {"is_final",    T_BOOL,      offsetof(JPy_JField, isFinal),    READONLY, "Tests if this is a final field"},
    {NULL}  /* Sentinel */
};


/**
 * Implements the BeamPy_JObjectType class singleton.
 */
PyTypeObject JField_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "jpy.JField",                 /* tp_name */
    sizeof (JPy_JField),          /* tp_basicsize */
    0,                            /* tp_itemsize */
    (destructor)JField_dealloc,   /* tp_dealloc */
    NULL,                         /* tp_print */
    NULL,                         /* tp_getattr */
    NULL,                         /* tp_setattr */
    NULL,                         /* tp_reserved */
    (reprfunc)JField_repr,        /* tp_repr */
    NULL,                         /* tp_as_number */
    NULL,                         /* tp_as_sequence */
    NULL,                         /* tp_as_mapping */
    NULL,                         /* tp_hash  */
    NULL,                         /* tp_call */
    (reprfunc)JField_str,         /* tp_str */
    NULL,                         /* tp_getattro */
    NULL,                         /* tp_setattro */
    NULL,                         /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,           /* tp_flags */
    "Java Field Wrapper",         /* tp_doc */
    NULL,                         /* tp_traverse */
    NULL,                         /* tp_clear */
    NULL,                         /* tp_richcompare */
    0,                            /* tp_weaklistoffset */
    NULL,                         /* tp_iter */
    NULL,                         /* tp_iternext */
    NULL,                         /* tp_methods */
    JField_members,               /* tp_members */
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