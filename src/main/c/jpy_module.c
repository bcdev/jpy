#include "jpy_module.h"
#include "jpy_jtype.h"
#include "jpy_jmethod.h"
#include "jpy_jfield.h"
#include "jpy_jobj.h"
#include "jpy_carray.h"
#include "jpy_conv.h"

#include <stdlib.h>
#include <string.h>


PyObject* JPy_has_jvm(PyObject* self);
PyObject* JPy_create_jvm(PyObject* self, PyObject* args, PyObject* kwds);
PyObject* JPy_destroy_jvm(PyObject* self, PyObject* args);
PyObject* JPy_get_class(PyObject* self, PyObject* args, PyObject* kwds);
PyObject* JPy_cast(PyObject* self, PyObject* args);
PyObject* JPy_array(PyObject* self, PyObject* args);

static PyMethodDef JPy_Functions[] = {

    {"has_jvm",     (PyCFunction) JPy_has_jvm, METH_NOARGS,
                    "has_jvm() - Checks if the a JVM is available."},

    {"create_jvm",  (PyCFunction) JPy_create_jvm, METH_VARARGS|METH_KEYWORDS,
                    "create_jvm(options, debug=False) - Creates the Java VM from the given list of options."},

    {"destroy_jvm", JPy_destroy_jvm, METH_VARARGS,
                    "destroy_jvm() - Destroys the current Java VM."},

    {"get_class",   (PyCFunction) JPy_get_class, METH_VARARGS|METH_KEYWORDS,
                    "get_class(name, resolve=True) - Returns the Java class with the given name, e.g. 'java.io.File'. "
                    "Loads the Java class from the JVM if not already done. Optionally avoids resolving the class' methods."},

    {"cast",        JPy_cast, METH_VARARGS,
                    "cast(obj, type) - Caststhe given Java object to the given Java type. "
                    "Returns None if the cast is not possible."},

    {"array",       JPy_array, METH_VARARGS,
                    "array(name, length) - Returns a new Java array of given Java type and length. "
                    "Possible primitive types are 'boolean', 'byte', 'char', 'short', 'int', 'long', 'float', and 'double'."},

    {NULL, NULL, 0, NULL} /*Sentinel*/
};

static struct PyModuleDef JPy_ModuleDef =
{
    PyModuleDef_HEAD_INIT,
    "jpy",           /* Name of the Python JPy_Module */
    "Java Python Bridge",  /* Module documentation */
    -1,                 /* Size of per-interpreter state of the JPy_Module, or -1 if the JPy_Module keeps state in global variables. */
    JPy_Functions,    /* Structure containing global jpy-functions */
    NULL,   // m_reload
    NULL,   // m_traverse
    NULL,   // m_clear
    NULL    // m_free
};

PyObject* JPy_Module = NULL;
PyObject* JPy_Types = NULL;
PyObject* JPy_Type_Callbacks = NULL;
PyObject* JException_Type = NULL;

typedef struct {
    // A global reference to a Java VM.
    JavaVM* jvm;
    // The current JNI interface.
    JNIEnv* jenv;
    // If true, this JVM structure has been created from Python jpy.create_jvm()
    jboolean mustDestroy;
    // Used for switching debug prints in this module
    jboolean debug;
} JPy_JVM;

static JPy_JVM JVM = {NULL};

#define JPY_JNI_VERSION JNI_VERSION_1_6


// Global VM Information (todo: put this into JPy_JVM structure)
// {{{

JPy_JType* JPy_JBoolean = NULL;
JPy_JType* JPy_JChar = NULL;
JPy_JType* JPy_JByte = NULL;
JPy_JType* JPy_JShort = NULL;
JPy_JType* JPy_JInt = NULL;
JPy_JType* JPy_JLong = NULL;
JPy_JType* JPy_JFloat = NULL;
JPy_JType* JPy_JDouble = NULL;
JPy_JType* JPy_JVoid = NULL;
JPy_JType* JPy_JBooleanObj = NULL;
JPy_JType* JPy_JCharacterObj = NULL;
JPy_JType* JPy_JByteObj = NULL;
JPy_JType* JPy_JShortObj = NULL;
JPy_JType* JPy_JIntegerObj = NULL;
JPy_JType* JPy_JLongObj = NULL;
JPy_JType* JPy_JFloatObj = NULL;
JPy_JType* JPy_JDoubleObj = NULL;
JPy_JType* JPy_JString = NULL;



// java.lang.Comparable
jclass JPy_Comparable_JClass = NULL;

// java.lang.Object
jclass JPy_Object_JClass = NULL;
jmethodID JPy_Object_ToString_MID = NULL;
jmethodID JPy_Object_HashCode_MID = NULL;
jmethodID JPy_Object_Equals_MID = NULL;

// java.lang.Class
jclass JPy_Class_JClass = NULL;
jmethodID JPy_Class_GetName_MID = NULL;
jmethodID JPy_Class_GetDeclaredConstructors_MID = NULL;
jmethodID JPy_Class_GetDeclaredFields_MID = NULL;
jmethodID JPy_Class_GetDeclaredMethods_MID = NULL;
jmethodID JPy_Class_GetComponentType_MID = NULL;
jmethodID JPy_Class_IsPrimitive_MID = NULL;

// java.lang.reflect.Constructor
jclass JPy_Constructor_JClass = NULL;
jmethodID JPy_Constructor_GetModifiers_MID = NULL;
jmethodID JPy_Constructor_GetParameterTypes_MID = NULL;

// java.lang.reflect.Method
jclass JPy_Method_JClass = NULL;
jmethodID JPy_Method_GetName_MID = NULL;
jmethodID JPy_Method_GetReturnType_MID = NULL;
jmethodID JPy_Method_GetParameterTypes_MID = NULL;
jmethodID JPy_Method_GetModifiers_MID = NULL;

// java.lang.reflect.Field
jclass JPy_Field_JClass = NULL;
jmethodID JPy_Field_GetName_MID = NULL;
jmethodID JPy_Field_GetModifiers_MID = NULL;
jmethodID JPy_Field_GetType_MID = NULL;

// java.lang.Boolean
jclass JPy_Boolean_JClass = NULL;
jmethodID JPy_Boolean_Init_MID = NULL;
jmethodID JPy_Boolean_BooleanValue_MID = NULL;

jclass JPy_Character_JClass = NULL;
jmethodID JPy_Character_Init_MID;
jmethodID JPy_Character_CharValue_MID = NULL;

jclass JPy_Byte_JClass = NULL;
jmethodID JPy_Byte_Init_MID = NULL;

jclass JPy_Short_JClass = NULL;
jmethodID JPy_Short_Init_MID = NULL;

jclass JPy_Integer_JClass = NULL;
jmethodID JPy_Integer_Init_MID = NULL;

jclass JPy_Long_JClass = NULL;
jmethodID JPy_Long_Init_MID = NULL;

jclass JPy_Float_JClass = NULL;
jmethodID JPy_Float_Init_MID = NULL;

jclass JPy_Double_JClass = NULL;
jmethodID JPy_Double_Init_MID = NULL;

// java.lang.Number
jclass JPy_Number_JClass = NULL;
jmethodID JPy_Number_IntValue_MID = NULL;
jmethodID JPy_Number_LongValue_MID = NULL;
jmethodID JPy_Number_DoubleValue_MID = NULL;

// }}}


int JPy_InitGlobalVars(JNIEnv* jenv);
void JPy_ClearGlobalVars(void);


/**
 * Called if the JVM loads this module.
 * Will only called if this module's code is linked into a shared library and loaded by a Java VM.
 */
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* jvm, void* reserved)
{
    JNIEnv* jenv = NULL;

    if (JVM.jvm == NULL) {
        (*jvm)->GetEnv(jvm, (void**) &jenv, JPY_JNI_VERSION);
        JVM.jvm = jvm;
        JVM.jenv = jenv;
        JVM.mustDestroy = JNI_FALSE;
    } else if (JVM.jvm == jvm) {
        //if (JVM.debug)
            printf("jpy: JNI_OnLoad: same JVM already running\n");
    } else {
        //if (JVM.debug)
            printf("jpy: JNI_OnLoad: different JVM already running (expect weird things!)\n");
    }

    printf("jpy: JNI_OnLoad: JVM.jvm=%p, JVM.jenv=%p, JVM.mustDestroy=%d, JVM.debug=%d\n", JVM.jvm, JVM.jenv, JVM.mustDestroy, JVM.debug);

    return JPY_JNI_VERSION;
}

/**
 * Called if the JVM unloads this module.
 * Will only called if this module's code is linked into a shared library and loaded by a Java VM.
 */
JNIEXPORT void JNICALL JNI_OnUnload(JavaVM* jvm, void* reserved)
{
    printf("jpy: JNI_OnUnload: JVM.jvm=%p, JVM.jenv=%p, JVM.mustDestroy=%d, JVM.debug=%d\n", JVM.jvm, JVM.jenv, JVM.mustDestroy, JVM.debug);

    if (!JVM.mustDestroy) {
        JVM.jvm = NULL;
        JVM.jenv = NULL;
        JPy_ClearGlobalVars();
    }
}

JNIEnv* JPy_GetJNIEnv(void)
{
    // Currently JPy is only single threaded.
    // To make it multi-threaded we must use the following code:
    // (*JVM.jvm)->AttachCurrentThread(JVM.jvm, &JVM.jenv, NULL);
    return JVM.jenv;
}

jboolean JPy_IsDebug(void)
{
    return JVM.debug;
}

void JPy_SetDebug(jboolean debug)
{
    JVM.debug = debug;
}


/**
 * Called by the Python interpreter's import machinery, e.g. using 'import jpy'.
 */
PyMODINIT_FUNC PyInit_jpy(void)
{
    printf("PyInit_jpy: JVM.jvm=%p, JVM.jenv=%p, JVM.debug=%d\n", JVM.jvm, JVM.jenv, JVM.debug);

    /////////////////////////////////////////////////////////////////////////

    JPy_Module = PyModule_Create(&JPy_ModuleDef);
    if (JPy_Module == NULL) {
        return NULL;
    }

    /////////////////////////////////////////////////////////////////////////

    if (PyType_Ready(&CArray_Type) < 0) {
        return NULL;
    }
    Py_INCREF(&CArray_Type);
    PyModule_AddObject(JPy_Module, "CArray", (PyObject*) &CArray_Type);

    /////////////////////////////////////////////////////////////////////////

    if (PyType_Ready(&JType_Type) < 0) {
        return NULL;
    }
    Py_INCREF(&JType_Type);
    PyModule_AddObject(JPy_Module, "JType", (PyObject*) &JType_Type);

    /////////////////////////////////////////////////////////////////////////

    if (PyType_Ready(&JMethod_Type) < 0) {
        return NULL;
    }
    Py_INCREF(&JMethod_Type);
    PyModule_AddObject(JPy_Module, "JMethod", (PyObject*) &JMethod_Type);

    /////////////////////////////////////////////////////////////////////////

    if (PyType_Ready(&JOverloadedMethod_Type) < 0) {
        return NULL;
    }
    Py_INCREF(&JOverloadedMethod_Type);
    PyModule_AddObject(JPy_Module, "JOverloadedMethod", (PyObject*) &JOverloadedMethod_Type);

    /////////////////////////////////////////////////////////////////////////

    if (PyType_Ready(&JField_Type) < 0) {
        return NULL;
    }
    Py_INCREF(&JField_Type);
    PyModule_AddObject(JPy_Module, "JField", (PyObject*) &JField_Type);

    /////////////////////////////////////////////////////////////////////////

    JException_Type = PyErr_NewException("jpy.JException", NULL, NULL);
    Py_INCREF(JException_Type);
    PyModule_AddObject(JPy_Module, "JException", JException_Type);

    /////////////////////////////////////////////////////////////////////////

    JPy_Types = PyDict_New();
    Py_INCREF(JPy_Types);
    PyModule_AddObject(JPy_Module, "types", JPy_Types);

    /////////////////////////////////////////////////////////////////////////

    JPy_Type_Callbacks = PyDict_New();
    Py_INCREF(JPy_Type_Callbacks);
    PyModule_AddObject(JPy_Module, "type_callbacks", JPy_Type_Callbacks);

    /////////////////////////////////////////////////////////////////////////

    if (JVM.jenv != NULL) {
        // If we have already a running VM, initialize global variables
        if (JPy_InitGlobalVars(JVM.jenv) < 0) {
            return NULL;
        }
    }

    /////////////////////////////////////////////////////////////////////////

    //printf("PyInit_jpy: exit\n");

    return JPy_Module;
}

PyObject* JPy_has_jvm(PyObject* self)
{
    return PyBool_FromLong(JVM.jvm != NULL);
}

PyObject* JPy_create_jvm(PyObject* self, PyObject* args, PyObject* kwds)
{
    static char* keywords[] = {"options", "debug", NULL};
    PyObject*   options;
    Py_ssize_t  optionCount;
    PyObject*   option;
    JavaVMOption* jvmOptions;
    JavaVMInitArgs jvmInitArgs;
    Py_ssize_t  i;
    jint        res;
    int         debug;

    //printf("JPy_create_jvm: JVM.jvm=%p, JVM.jenv=%p, JVM.debug=%d\n", JVM.jvm, JVM.jenv, JVM.debug);

    options = NULL;
    debug = 0;
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O|p:create_jvm", keywords, &options, &debug)) {
        return NULL;
    }

    if (JVM.jvm != NULL) {
        if (debug) printf("jpy: Java VM is already running.\n");
        Py_DECREF(options);
        return Py_BuildValue("");
    }

    if (!PySequence_Check(options)) {
        PyErr_SetString(PyExc_ValueError, "list of JVM options expected");
        return NULL;
    }

    optionCount = PySequence_Length(options);
    if (optionCount == -1) {
        PyErr_SetString(PyExc_ValueError, "can't retrieve number of JVM options");
        return NULL;
    }

    jvmOptions = PyMem_New(JavaVMOption, optionCount);
    if (jvmOptions == NULL) {
        return PyErr_NoMemory();
    }

    for (i = 0; i < optionCount; i++) {
        option = PySequence_GetItem(options, i);
        if (option == NULL) {
            PyMem_Del(jvmOptions);
            return NULL;
        }
        jvmOptions[i].optionString = PyUnicode_AsUTF8(option);
        //printf("jvmOptions[%d].optionString = '%s'\n", i, jvmOptions[i].optionString);
        if (jvmOptions[i].optionString == NULL) {
            PyMem_Del(jvmOptions);
            return NULL;
        }
        Py_DECREF(option);
    }

    jvmInitArgs.version = JPY_JNI_VERSION;
    jvmInitArgs.options = jvmOptions;
    jvmInitArgs.nOptions = (size_t) optionCount;
    jvmInitArgs.ignoreUnrecognized = 0;
    res = JNI_CreateJavaVM(&JVM.jvm, (void**) &JVM.jenv, &jvmInitArgs);
    JVM.mustDestroy = JNI_TRUE;
    JVM.debug = debug;

    if (JPy_IsDebug()) printf("JPy_create_jvm: res=%d, JVM.jvm=%p, JVM.jenv=%p\n", res, JVM.jvm, JVM.jenv);

    PyMem_Del(jvmOptions);

    if (res != JNI_OK) {
        char msg[1024];
        sprintf(msg, "failed to create JVM: JNI_CreateJavaVM() returned exit code %d. "
                      "Make sure the JVM shared library (Unix: libjvm.so, Windows: jvm.dll) can be found. "
                      "Check your 'path' environment variable.", res);
        PyErr_SetString(PyExc_ValueError, msg);
        return NULL;
    }

    if (JPy_InitGlobalVars(JVM.jenv) < 0) {
        return NULL;
    }

    return Py_BuildValue("");
}

PyObject* JPy_destroy_jvm(PyObject* self, PyObject* args)
{
    if (JPy_IsDebug()) printf("JPy_destroy_jvm: JVM.jvm=%p, JVM.jenv=%p\n", JVM.jvm, JVM.jenv);

    if (JVM.jvm != NULL && JVM.mustDestroy) {
        (*JVM.jvm)->DestroyJavaVM(JVM.jvm);
        JVM.jvm = NULL;
        JVM.jenv = NULL;
        JPy_ClearGlobalVars();
    }

    return Py_BuildValue("");
}

PyObject* JPy_get_class(PyObject* self, PyObject* args, PyObject* kwds)
{
    JNIEnv* jenv;
    static char* keywords[] = {"name", "resolve", NULL};
    const char* className;
    int resolve;

    JPy_GET_JNI_ENV_OR_RETURN(jenv, NULL)

    resolve = JNI_TRUE;
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "s|p:get_class", keywords, &className, &resolve)) {
        return NULL;
    }

    return (PyObject*) JType_GetTypeForName(jenv, className, (jboolean) resolve);
}

PyObject* JPy_cast(PyObject* self, PyObject* args)
{
    JNIEnv* jenv;
    PyObject* obj;
    PyObject* objType;
    jboolean inst;

    JPy_GET_JNI_ENV_OR_RETURN(jenv, NULL)

    if (!PyArg_ParseTuple(args, "OO:cast", &obj, &objType)) {
        return NULL;
    }

    if (obj == Py_None) {
        return Py_BuildValue("");
    }

    if (!JObj_Check(obj)) {
        PyErr_SetString(PyExc_ValueError, "argument 1 (obj) must be a Java object");
        return NULL;
    }

    if (!JType_Check(objType)) {
        PyErr_SetString(PyExc_ValueError, "argument 2 (obj_type) must be a Java type object");
        return NULL;
    }

    inst = (*jenv)->IsInstanceOf(jenv, ((JPy_JObj*) obj)->objectRef, ((JPy_JType*) objType)->classRef);
    if (inst) {
        return (PyObject*) JObj_FromType(jenv, (JPy_JType*) objType, ((JPy_JObj*) obj)->objectRef);
    } else {
        return Py_BuildValue("");
    }
}

PyObject* JPy_array(PyObject* self, PyObject* args)
{
    JNIEnv* jenv;
    JPy_JType* type;
    const char* name;
    int length;
    jarray arrayRef;
    jclass classRef;

    JPy_GET_JNI_ENV_OR_RETURN(jenv, NULL)

    if (!PyArg_ParseTuple(args, "si:array", &name, &length)) {
        return NULL;
    }

    if (length < 0) {
        PyErr_SetString(PyExc_ValueError, "negative array length");
    }

    type = NULL;
    if (strcmp(name, "boolean") == 0) {
        arrayRef = (*jenv)->NewBooleanArray(jenv, length);
    } else if (strcmp(name, "byte") == 0) {
        arrayRef = (*jenv)->NewByteArray(jenv, length);
    } else if (strcmp(name, "char") == 0) {
        arrayRef = (*jenv)->NewCharArray(jenv, length);
    } else if (strcmp(name, "short") == 0) {
        arrayRef = (*jenv)->NewShortArray(jenv, length);
    } else if (strcmp(name, "int") == 0) {
        arrayRef = (*jenv)->NewIntArray(jenv, length);
    } else if (strcmp(name, "long") == 0) {
        arrayRef = (*jenv)->NewLongArray(jenv, length);
    } else if (strcmp(name, "float") == 0) {
        arrayRef = (*jenv)->NewFloatArray(jenv, length);
    } else if (strcmp(name, "double") == 0) {
        arrayRef = (*jenv)->NewDoubleArray(jenv, length);
    } else {
        type = JType_GetTypeForName(jenv, name, JNI_FALSE);
        if (type == NULL) {
            return NULL;
        }
        arrayRef = (*jenv)->NewObjectArray(jenv, length, ((JPy_JType*) type)->classRef, NULL);
    }

    if (arrayRef == NULL) {
        return PyErr_NoMemory();
    }

    classRef = (*jenv)->GetObjectClass(jenv, arrayRef);
    type = JType_GetType(jenv, classRef, JNI_FALSE);
    if (type == NULL) {
        return NULL;
    }

    return (PyObject*) JObj_FromType(jenv, type, arrayRef);
}

JPy_JType* JPy_GetNonObjectJType(JNIEnv* jenv, const char* className)
{
    jclass classRef;
    jclass primClassRef;
    jfieldID fid;
    JPy_JType* type;

    classRef = (*jenv)->FindClass(jenv, className);
    fid = (*jenv)->GetStaticFieldID(jenv, classRef, "TYPE", "Ljava/lang/Class;");
    primClassRef = (*jenv)->GetStaticObjectField(jenv, classRef, fid);
    type = JType_GetType(jenv, primClassRef, JNI_FALSE);
    type->isResolved = JNI_TRUE; // Primitive types are always resolved.
    Py_INCREF((PyObject*) type);

    return type;
}

JPy_JType* JPy_GetObjectJType(JNIEnv* jenv, const char* className)
{
    jclass classRef;
    JPy_JType* type;

    classRef = (*jenv)->FindClass(jenv, className);
    type = JType_GetType(jenv, classRef, JNI_TRUE);

    return type;
}

#define DEFINE_NON_OBJECT_TYPE(T, N) \
    T = JPy_GetNonObjectJType(jenv, N); \
    if (T == NULL) { \
        return -1; \
    }

#define DEFINE_OBJECT_TYPE(T, N) \
    T = JPy_GetObjectJType(jenv, N); \
    if (T == NULL) { \
        return -1; \
    }


int JPy_InitGlobalVars(JNIEnv* jenv)
{
    if (JPy_Comparable_JClass != NULL) {
        return 0;
    }

    // todo: check, if we need to convert all jclass types using NewGlobalReference()

    JPy_Comparable_JClass = (*jenv)->FindClass(jenv, "java/lang/Comparable");

    JPy_Object_JClass = (*jenv)->FindClass(jenv, "java/lang/Object");
    JPy_Object_ToString_MID = (*jenv)->GetMethodID(jenv, JPy_Object_JClass, "toString", "()Ljava/lang/String;");
    JPy_Object_HashCode_MID = (*jenv)->GetMethodID(jenv, JPy_Object_JClass, "hashCode", "()I");
    JPy_Object_Equals_MID = (*jenv)->GetMethodID(jenv, JPy_Object_JClass, "equals", "(Ljava/lang/Object;)Z");

    JPy_Class_JClass = (*jenv)->FindClass(jenv, "java/lang/Class");
    JPy_Class_GetName_MID = (*jenv)->GetMethodID(jenv, JPy_Class_JClass, "getName", "()Ljava/lang/String;");
    JPy_Class_GetDeclaredConstructors_MID = (*jenv)->GetMethodID(jenv, JPy_Class_JClass, "getDeclaredConstructors", "()[Ljava/lang/reflect/Constructor;");
    JPy_Class_GetDeclaredMethods_MID = (*jenv)->GetMethodID(jenv, JPy_Class_JClass, "getDeclaredMethods", "()[Ljava/lang/reflect/Method;");
    JPy_Class_GetDeclaredFields_MID = (*jenv)->GetMethodID(jenv, JPy_Class_JClass, "getDeclaredFields", "()[Ljava/lang/reflect/Field;");
    JPy_Class_GetComponentType_MID = (*jenv)->GetMethodID(jenv, JPy_Class_JClass, "getComponentType", "()Ljava/lang/Class;");
    JPy_Class_IsPrimitive_MID = (*jenv)->GetMethodID(jenv, JPy_Class_JClass, "isPrimitive", "()Z");

    JPy_Constructor_JClass = (*jenv)->FindClass(jenv, "java/lang/reflect/Constructor");
    JPy_Constructor_GetModifiers_MID = (*jenv)->GetMethodID(jenv, JPy_Constructor_JClass, "getModifiers", "()I");
    JPy_Constructor_GetParameterTypes_MID = (*jenv)->GetMethodID(jenv, JPy_Constructor_JClass, "getParameterTypes", "()[Ljava/lang/Class;");

    JPy_Field_JClass = (*jenv)->FindClass(jenv, "java/lang/reflect/Field");
    JPy_Field_GetName_MID = (*jenv)->GetMethodID(jenv, JPy_Field_JClass, "getName", "()Ljava/lang/String;");
    JPy_Field_GetModifiers_MID = (*jenv)->GetMethodID(jenv, JPy_Field_JClass, "getModifiers", "()I");
    JPy_Field_GetType_MID = (*jenv)->GetMethodID(jenv, JPy_Field_JClass, "getType", "()Ljava/lang/Class;");

    JPy_Method_JClass = (*jenv)->FindClass(jenv, "java/lang/reflect/Method");
    JPy_Method_GetName_MID = (*jenv)->GetMethodID(jenv, JPy_Method_JClass, "getName", "()Ljava/lang/String;");
    JPy_Method_GetModifiers_MID = (*jenv)->GetMethodID(jenv, JPy_Method_JClass, "getModifiers", "()I");
    JPy_Method_GetParameterTypes_MID = (*jenv)->GetMethodID(jenv, JPy_Method_JClass, "getParameterTypes", "()[Ljava/lang/Class;");
    JPy_Method_GetReturnType_MID = (*jenv)->GetMethodID(jenv, JPy_Method_JClass, "getReturnType", "()Ljava/lang/Class;");

    // java.lang.Boolean
    JPy_Boolean_JClass = (*jenv)->FindClass(jenv, "java/lang/Boolean");
    JPy_Boolean_Init_MID = (*jenv)->GetMethodID(jenv, JPy_Boolean_JClass, "<init>", "(Z)V");
    JPy_Boolean_BooleanValue_MID = (*jenv)->GetMethodID(jenv, JPy_Boolean_JClass, "booleanValue", "()Z");

    JPy_Character_JClass = (*jenv)->FindClass(jenv, "java/lang/Character");
    JPy_Character_Init_MID = (*jenv)->GetMethodID(jenv, JPy_Character_JClass, "<init>", "(C)V");
    JPy_Character_CharValue_MID = (*jenv)->GetMethodID(jenv, JPy_Character_JClass, "charValue", "()C");

    JPy_Byte_JClass = (*jenv)->FindClass(jenv, "java/lang/Byte");
    JPy_Byte_Init_MID = (*jenv)->GetMethodID(jenv, JPy_Byte_JClass, "<init>", "(B)V");

    JPy_Short_JClass = (*jenv)->FindClass(jenv, "java/lang/Short");
    JPy_Short_Init_MID = (*jenv)->GetMethodID(jenv, JPy_Short_JClass, "<init>", "(S)V");

    JPy_Integer_JClass = (*jenv)->FindClass(jenv, "java/lang/Integer");
    JPy_Integer_Init_MID = (*jenv)->GetMethodID(jenv, JPy_Integer_JClass, "<init>", "(I)V");

    JPy_Long_JClass = (*jenv)->FindClass(jenv, "java/lang/Long");
    JPy_Long_Init_MID = (*jenv)->GetMethodID(jenv, JPy_Long_JClass, "<init>", "(J)V");

    JPy_Float_JClass = (*jenv)->FindClass(jenv, "java/lang/Float");
    JPy_Float_Init_MID = (*jenv)->GetMethodID(jenv, JPy_Float_JClass, "<init>", "(F)V");

    JPy_Double_JClass = (*jenv)->FindClass(jenv, "java/lang/Double");
    JPy_Double_Init_MID = (*jenv)->GetMethodID(jenv, JPy_Double_JClass, "<init>", "(D)V");

    // java.lang.Number
    JPy_Number_JClass = (*jenv)->FindClass(jenv, "java/lang/Number");
    JPy_Number_IntValue_MID = (*jenv)->GetMethodID(jenv, JPy_Number_JClass, "intValue", "()I");
    JPy_Number_LongValue_MID  = (*jenv)->GetMethodID(jenv, JPy_Number_JClass, "longValue", "()J");
    JPy_Number_DoubleValue_MID  = (*jenv)->GetMethodID(jenv, JPy_Number_JClass, "doubleValue", "()D");


    DEFINE_NON_OBJECT_TYPE(JPy_JBoolean, "java/lang/Boolean");
    DEFINE_NON_OBJECT_TYPE(JPy_JByte, "java/lang/Byte");
    DEFINE_NON_OBJECT_TYPE(JPy_JChar, "java/lang/Character");
    DEFINE_NON_OBJECT_TYPE(JPy_JShort, "java/lang/Short");
    DEFINE_NON_OBJECT_TYPE(JPy_JInt, "java/lang/Integer");
    DEFINE_NON_OBJECT_TYPE(JPy_JLong, "java/lang/Long");
    DEFINE_NON_OBJECT_TYPE(JPy_JFloat, "java/lang/Float");
    DEFINE_NON_OBJECT_TYPE(JPy_JDouble, "java/lang/Double");
    DEFINE_NON_OBJECT_TYPE(JPy_JVoid, "java/lang/Void");

    DEFINE_OBJECT_TYPE(JPy_JBooleanObj, "java/lang/Boolean");
    DEFINE_OBJECT_TYPE(JPy_JByteObj, "java/lang/Byte");
    DEFINE_OBJECT_TYPE(JPy_JCharacterObj, "java/lang/Character");
    DEFINE_OBJECT_TYPE(JPy_JShortObj, "java/lang/Short");
    DEFINE_OBJECT_TYPE(JPy_JIntegerObj, "java/lang/Integer");
    DEFINE_OBJECT_TYPE(JPy_JLongObj, "java/lang/Long");
    DEFINE_OBJECT_TYPE(JPy_JFloatObj, "java/lang/Float");
    DEFINE_OBJECT_TYPE(JPy_JDoubleObj, "java/lang/Double");
    DEFINE_OBJECT_TYPE(JPy_JString, "java/lang/String");

    return 0;
}

void JPy_ClearGlobalVars(void)
{
    Py_DECREF(JPy_JBoolean);
    Py_DECREF(JPy_JByte);
    Py_DECREF(JPy_JShort);
    Py_DECREF(JPy_JInt);
    Py_DECREF(JPy_JLong);
    Py_DECREF(JPy_JFloat);
    Py_DECREF(JPy_JDouble);
    Py_DECREF(JPy_JChar);
    Py_DECREF(JPy_JVoid);

    JPy_JBoolean = NULL;
    JPy_JByte = NULL;
    JPy_JShort = NULL;
    JPy_JInt = NULL;
    JPy_JLong = NULL;
    JPy_JFloat = NULL;
    JPy_JDouble = NULL;
    JPy_JChar = NULL;
    JPy_JVoid = NULL;

    JPy_Comparable_JClass = NULL;

    JPy_Object_JClass = NULL;
    JPy_Object_ToString_MID = NULL;
    JPy_Object_HashCode_MID = NULL;
    JPy_Object_Equals_MID = NULL;

    JPy_Class_JClass = NULL;
    JPy_Class_GetName_MID = NULL;
    JPy_Class_GetDeclaredConstructors_MID = NULL;
    JPy_Class_GetDeclaredFields_MID = NULL;
    JPy_Class_GetDeclaredMethods_MID = NULL;
    JPy_Class_GetComponentType_MID = NULL;
    JPy_Class_IsPrimitive_MID = NULL;

    JPy_Constructor_JClass = NULL;
    JPy_Constructor_GetModifiers_MID = NULL;
    JPy_Constructor_GetParameterTypes_MID = NULL;

    JPy_Method_JClass = NULL;
    JPy_Method_GetName_MID = NULL;
    JPy_Method_GetReturnType_MID = NULL;
    JPy_Method_GetParameterTypes_MID = NULL;
    JPy_Method_GetModifiers_MID = NULL;

    // java.lang.reflect.Field
    JPy_Field_JClass = NULL;
    JPy_Field_GetName_MID = NULL;
    JPy_Field_GetModifiers_MID = NULL;
    JPy_Field_GetType_MID = NULL;
}