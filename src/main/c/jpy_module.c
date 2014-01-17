#include "jpy_module.h"
#include "jpy_jtype.h"
#include "jpy_jmethod.h"
#include "jpy_jfield.h"
#include "jpy_jobj.h"
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
                    "cast(obj, type) - Casts the given Java object to the given Java type. "
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
    // If true, this JVM structure has been created from Python jpy.create_jvm()
    jboolean mustDestroy;
    // Used for switching debug prints in this module
    jboolean debug;
} JPy_JVM;

static JPy_JVM JVM = {NULL};

#define JPY_JNI_VERSION JNI_VERSION_1_6


// Global VM Information (maybe better place this in the JPy_JVM structure later)
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
JPy_JType* JPy_JObject = NULL;
JPy_JType* JPy_JString = NULL;



// java.lang.Comparable
jclass JPy_Comparable_JClass = NULL;
jmethodID JPy_Comparable_CompareTo_MID = NULL;

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

jclass JPy_RuntimeException_JClass = NULL;

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

jclass JPy_Void_JClass = NULL;
jclass JPy_String_JClass = NULL;

// }}}


int JPy_InitGlobalVars(JNIEnv* jenv);
void JPy_ClearGlobalVars(void);


/**
 * Called if the JVM loads this module.
 * Will only called if this module's code is linked into a shared library and loaded by a Java VM.
 */
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* jvm, void* reserved)
{
    if (JVM.jvm == NULL) {
        JVM.jvm = jvm;
        JVM.mustDestroy = JNI_FALSE;
    } else if (JVM.jvm == jvm) {
        //if (JVM.debug)
        //    printf("jpy: JNI_OnLoad: same JVM already running\n");
    } else {
        //if (JVM.debug)
        //    printf("jpy: JNI_OnLoad: different JVM already running (expect weird things!)\n");
    }

    printf("jpy: JNI_OnLoad: JVM.jvm=%p, JVM.mustDestroy=%d, JVM.debug=%d\n", JVM.jvm, JVM.mustDestroy, JVM.debug);

    return JPY_JNI_VERSION;
}

/**
 * Called if the JVM unloads this module.
 * Will only called if this module's code is linked into a shared library and loaded by a Java VM.
 */
JNIEXPORT void JNICALL JNI_OnUnload(JavaVM* jvm, void* reserved)
{
    printf("jpy: JNI_OnUnload: JVM.jvm=%p, JVM.mustDestroy=%d, JVM.debug=%d\n", JVM.jvm, JVM.mustDestroy, JVM.debug);

    if (!JVM.mustDestroy) {
        JVM.jvm = NULL;
        JPy_ClearGlobalVars();
    }
}

JNIEnv* JPy_GetJNIEnv(void)
{
    JavaVM* jvm;
    JNIEnv* jenv;
    jint status;

    jvm = JVM.jvm;
    if (jvm == NULL) {
        PyErr_SetString(PyExc_RuntimeError, "jpy: No JVM available.");
        return NULL;
    }

    status = (*jvm)->GetEnv(jvm, (void**) &jenv, JPY_JNI_VERSION);
    if (status == JNI_EDETACHED) {
        if (JVM.debug) printf("jpy: Attaching current thread to JVM.\n");
        if ((*jvm)->AttachCurrentThread(jvm, (void**) &jenv, NULL) != 0) {
            PyErr_SetString(PyExc_RuntimeError, "jpy: Failed to attach current thread to JVM.");
            return NULL;
        }
    } else if (status == JNI_EVERSION) {
        PyErr_SetString(PyExc_RuntimeError, "jpy: Failed to attach current thread to JVM: Java version not supported.");
        return NULL;
    } else if (status == JNI_OK) {
        // ok!
    }

    return jenv;
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
    //printf("PyInit_jpy: JVM.jvm=%p, JVM.jenv=%p, JVM.debug=%d\n", JVM.jvm, JVM.jenv, JVM.debug);

    /////////////////////////////////////////////////////////////////////////

    JPy_Module = PyModule_Create(&JPy_ModuleDef);
    if (JPy_Module == NULL) {
        return NULL;
    }

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
    PyModule_AddObject(JPy_Module, JPy_MODULE_ATTR_NAME_TYPES, JPy_Types);

    /////////////////////////////////////////////////////////////////////////

    JPy_Type_Callbacks = PyDict_New();
    Py_INCREF(JPy_Type_Callbacks);
    PyModule_AddObject(JPy_Module, JPy_MODULE_ATTR_NAME_TYPE_CALLBACKS, JPy_Type_Callbacks);

    /////////////////////////////////////////////////////////////////////////

    if (JVM.jvm != NULL) {
        JNIEnv* jenv;
        jenv = JPy_GetJNIEnv();
        if (jenv == NULL) {
            return NULL;
        }
        // If we have already a running VM, initialize global variables
        if (JPy_InitGlobalVars(jenv) < 0) {
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
    jint        jvmErrorCode;
    JNIEnv*     jenv;
    Py_ssize_t  i;
    int         debug;

    //printf("JPy_create_jvm: JVM.jvm=%p, JVM.jenv=%p, JVM.debug=%d\n", JVM.jvm, JVM.jenv, JVM.debug);

    options = NULL;
    debug = 0;
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O|p:create_jvm", keywords, &options, &debug)) {
        return NULL;
    }

    if (JVM.jvm != NULL) {
        if (debug) printf("JPy_create_jvm: Java VM is already running.\n");
        Py_DECREF(options);
        return Py_BuildValue("");
    }

    if (!PySequence_Check(options)) {
        PyErr_SetString(PyExc_ValueError, "list of Java VM options expected");
        return NULL;
    }

    optionCount = PySequence_Length(options);
    if (optionCount == -1) {
        PyErr_SetString(PyExc_ValueError, "can't retrieve number of Java VM options");
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
        if (debug) printf("JPy_create_jvm: jvmOptions[%d].optionString = '%s'\n", i, jvmOptions[i].optionString);
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
    jvmErrorCode = JNI_CreateJavaVM(&JVM.jvm, (void**) &jenv, &jvmInitArgs);
    JVM.mustDestroy = JNI_TRUE;
    JVM.debug = debug;

    JPy_DEBUG_PRINTF("JPy_create_jvm: res=%d, JVM.jvm=%p, jenv=%p\n", jvmErrorCode, JVM.jvm, jenv);

    PyMem_Del(jvmOptions);

    if (jvmErrorCode != JNI_OK) {
        PyErr_SetString(PyExc_RuntimeError, "failed to create Java VM");
        fprintf(stderr,
                "Failed to create Java VM (JNI error code %d). Possible reasons are:\n"
                "* The Java heap space setting is too high (option -Xmx). Try '256M' first, then increment.\n"
                "* The JVM shared library (Unix: libjvm.so, Windows: jvm.dll) cannot be found or cannot be loaded.\n"
                "  Make sure the shared library can be found via the 'PATH' environment variable.\n"
                "  Also make sure that the JVM is compiled for the same architecture as Python.\n",
                jvmErrorCode);
        return NULL;
    }

    if (JPy_InitGlobalVars(jenv) < 0) {
        return NULL;
    }

    return Py_BuildValue("");
}

PyObject* JPy_destroy_jvm(PyObject* self, PyObject* args)
{
    JPy_DEBUG_PRINTF("JPy_destroy_jvm: JVM.jvm=%p\n", JVM.jvm);

    if (JVM.jvm != NULL && JVM.mustDestroy) {
        (*JVM.jvm)->DestroyJavaVM(JVM.jvm);
        JVM.jvm = NULL;
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
    (*jenv)->DeleteLocalRef(jenv, classRef);
    if (type == NULL) {
        return NULL;
    }

    return (PyObject*) JObj_FromType(jenv, type, arrayRef);
}

JPy_JType* JPy_GetNonObjectJType(JNIEnv* jenv, jclass classRef)
{
    jclass primClassRef;
    jfieldID fid;
    JPy_JType* type;

    if (classRef == NULL) {
        PyErr_SetString(PyExc_RuntimeError, "internal error: classRef == NULL");
    }

    fid = (*jenv)->GetStaticFieldID(jenv, classRef, "TYPE", "Ljava/lang/Class;");
    if (fid == NULL) {
        PyErr_SetString(PyExc_RuntimeError, "field 'TYPE' not found");
        return NULL;
    }

    primClassRef = (*jenv)->GetStaticObjectField(jenv, classRef, fid);
    if (primClassRef == NULL) {
        PyErr_SetString(PyExc_RuntimeError, "failed to access field 'TYPE'");
        return NULL;
    }

    type = JType_GetType(jenv, primClassRef, JNI_FALSE);
    if (type == NULL) {
        return NULL;
    }

    type->isResolved = JNI_TRUE; // Primitive types are always resolved.
    Py_INCREF((PyObject*) type);

    return type;
}

jclass JPy_GetClass(JNIEnv* jenv, const char* name)
{
    jclass localClassRef;
    jclass globalClassRef;

    localClassRef = (*jenv)->FindClass(jenv, name);
    if (localClassRef == NULL) {
        PyErr_Format(PyExc_RuntimeError, "class '%s' not found", name);
        return NULL;
    }

    globalClassRef = (*jenv)->NewGlobalRef(jenv, localClassRef);
    (*jenv)->DeleteLocalRef(jenv, localClassRef);
    if (globalClassRef == NULL) {
        PyErr_NoMemory();
        return NULL;
    }

    return globalClassRef;
}

jmethodID JPy_GetMethod(JNIEnv* jenv, jclass classRef, const char* name, const char* sig)
{
    jmethodID methodID;
    methodID = (*jenv)->GetMethodID(jenv, classRef, name, sig);
    if (methodID == NULL) {
        PyErr_Format(PyExc_RuntimeError, "method not found: %s%s", name, sig);
        return NULL;
    }
    return methodID;
}



#define DEFINE_CLASS(C, N) \
    C = JPy_GetClass(jenv, N); \
    if (C == NULL) { \
        return -1; \
    }


#define DEFINE_METHOD(M, C, N, S) \
    M = JPy_GetMethod(jenv, C, N, S); \
    if (M == NULL) { \
        return -1; \
    }


#define DEFINE_NON_OBJECT_TYPE(T, C) \
    T = JPy_GetNonObjectJType(jenv, C); \
    if (T == NULL) { \
        return -1; \
    }


#define DEFINE_OBJECT_TYPE(T, C) \
    T = JType_GetType(jenv, C, JNI_FALSE); \
    if (T == NULL) { \
        return -1; \
    }


int JPy_InitGlobalVars(JNIEnv* jenv)
{
    if (JPy_Comparable_JClass != NULL) {
        return 0;
    }

    DEFINE_CLASS(JPy_Comparable_JClass, "java/lang/Comparable");
    DEFINE_METHOD(JPy_Comparable_CompareTo_MID, JPy_Comparable_JClass, "compareTo", "(Ljava/lang/Object;)I");

    DEFINE_CLASS(JPy_Object_JClass, "java/lang/Object");
    DEFINE_METHOD(JPy_Object_ToString_MID, JPy_Object_JClass, "toString", "()Ljava/lang/String;");
    DEFINE_METHOD(JPy_Object_HashCode_MID, JPy_Object_JClass, "hashCode", "()I");
    DEFINE_METHOD(JPy_Object_Equals_MID, JPy_Object_JClass, "equals", "(Ljava/lang/Object;)Z");

    DEFINE_CLASS(JPy_Class_JClass, "java/lang/Class");
    DEFINE_METHOD(JPy_Class_GetName_MID, JPy_Class_JClass, "getName", "()Ljava/lang/String;");
    DEFINE_METHOD(JPy_Class_GetDeclaredConstructors_MID, JPy_Class_JClass, "getDeclaredConstructors", "()[Ljava/lang/reflect/Constructor;");
    DEFINE_METHOD(JPy_Class_GetDeclaredMethods_MID, JPy_Class_JClass, "getDeclaredMethods", "()[Ljava/lang/reflect/Method;");
    DEFINE_METHOD(JPy_Class_GetDeclaredFields_MID, JPy_Class_JClass, "getDeclaredFields", "()[Ljava/lang/reflect/Field;");
    DEFINE_METHOD(JPy_Class_GetComponentType_MID, JPy_Class_JClass, "getComponentType", "()Ljava/lang/Class;");
    DEFINE_METHOD(JPy_Class_IsPrimitive_MID, JPy_Class_JClass, "isPrimitive", "()Z");

    DEFINE_CLASS(JPy_Constructor_JClass, "java/lang/reflect/Constructor");
    DEFINE_METHOD(JPy_Constructor_GetModifiers_MID, JPy_Constructor_JClass, "getModifiers", "()I");
    DEFINE_METHOD(JPy_Constructor_GetParameterTypes_MID, JPy_Constructor_JClass, "getParameterTypes", "()[Ljava/lang/Class;");

    DEFINE_CLASS(JPy_Field_JClass, "java/lang/reflect/Field");
    DEFINE_METHOD(JPy_Field_GetName_MID, JPy_Field_JClass, "getName", "()Ljava/lang/String;");
    DEFINE_METHOD(JPy_Field_GetModifiers_MID, JPy_Field_JClass, "getModifiers", "()I");
    DEFINE_METHOD(JPy_Field_GetType_MID, JPy_Field_JClass, "getType", "()Ljava/lang/Class;");

    DEFINE_CLASS(JPy_Method_JClass, "java/lang/reflect/Method");
    DEFINE_METHOD(JPy_Method_GetName_MID, JPy_Method_JClass, "getName", "()Ljava/lang/String;");
    DEFINE_METHOD(JPy_Method_GetModifiers_MID, JPy_Method_JClass, "getModifiers", "()I");
    DEFINE_METHOD(JPy_Method_GetParameterTypes_MID, JPy_Method_JClass, "getParameterTypes", "()[Ljava/lang/Class;");
    DEFINE_METHOD(JPy_Method_GetReturnType_MID, JPy_Method_JClass, "getReturnType", "()Ljava/lang/Class;");


    DEFINE_CLASS(JPy_RuntimeException_JClass, "java/lang/RuntimeException");


    DEFINE_CLASS(JPy_Boolean_JClass, "java/lang/Boolean");
    DEFINE_METHOD(JPy_Boolean_Init_MID, JPy_Boolean_JClass, "<init>", "(Z)V");
    DEFINE_METHOD(JPy_Boolean_BooleanValue_MID, JPy_Boolean_JClass, "booleanValue", "()Z");

    DEFINE_CLASS(JPy_Character_JClass, "java/lang/Character");
    DEFINE_METHOD(JPy_Character_Init_MID, JPy_Character_JClass, "<init>", "(C)V");
    DEFINE_METHOD(JPy_Character_CharValue_MID, JPy_Character_JClass, "charValue", "()C");

    DEFINE_CLASS(JPy_Byte_JClass, "java/lang/Byte");
    DEFINE_METHOD(JPy_Byte_Init_MID, JPy_Byte_JClass, "<init>", "(B)V");

    DEFINE_CLASS(JPy_Short_JClass, "java/lang/Short");
    DEFINE_METHOD(JPy_Short_Init_MID, JPy_Short_JClass, "<init>", "(S)V");

    DEFINE_CLASS(JPy_Integer_JClass, "java/lang/Integer");
    DEFINE_METHOD(JPy_Integer_Init_MID, JPy_Integer_JClass, "<init>", "(I)V");

    DEFINE_CLASS(JPy_Long_JClass, "java/lang/Long");
    DEFINE_METHOD(JPy_Long_Init_MID, JPy_Long_JClass, "<init>", "(J)V");

    DEFINE_CLASS(JPy_Float_JClass, "java/lang/Float");
    DEFINE_METHOD(JPy_Float_Init_MID, JPy_Float_JClass, "<init>", "(F)V");

    DEFINE_CLASS(JPy_Double_JClass, "java/lang/Double");
    DEFINE_METHOD(JPy_Double_Init_MID, JPy_Double_JClass, "<init>", "(D)V");

    DEFINE_CLASS(JPy_Number_JClass, "java/lang/Number");
    DEFINE_METHOD(JPy_Number_IntValue_MID, JPy_Number_JClass, "intValue", "()I");
    DEFINE_METHOD(JPy_Number_LongValue_MID , JPy_Number_JClass, "longValue", "()J");
    DEFINE_METHOD(JPy_Number_DoubleValue_MID, JPy_Number_JClass, "doubleValue", "()D");

    DEFINE_CLASS(JPy_Void_JClass, "java/lang/Void");

    DEFINE_CLASS(JPy_String_JClass, "java/lang/String");

    // Non-Object types: Primitive types and void.
    DEFINE_NON_OBJECT_TYPE(JPy_JBoolean, JPy_Boolean_JClass);
    DEFINE_NON_OBJECT_TYPE(JPy_JChar, JPy_Character_JClass);
    DEFINE_NON_OBJECT_TYPE(JPy_JByte, JPy_Byte_JClass);
    DEFINE_NON_OBJECT_TYPE(JPy_JShort, JPy_Short_JClass);
    DEFINE_NON_OBJECT_TYPE(JPy_JInt, JPy_Integer_JClass);
    DEFINE_NON_OBJECT_TYPE(JPy_JLong, JPy_Long_JClass);
    DEFINE_NON_OBJECT_TYPE(JPy_JFloat, JPy_Float_JClass);
    DEFINE_NON_OBJECT_TYPE(JPy_JDouble, JPy_Double_JClass);
    DEFINE_NON_OBJECT_TYPE(JPy_JVoid, JPy_Void_JClass);
    // The Java root object.
    DEFINE_OBJECT_TYPE(JPy_JObject, JPy_Object_JClass);
    // Primitive-Wrapper Objects.
    DEFINE_OBJECT_TYPE(JPy_JBooleanObj, JPy_Boolean_JClass);
    DEFINE_OBJECT_TYPE(JPy_JCharacterObj, JPy_Character_JClass);
    DEFINE_OBJECT_TYPE(JPy_JByteObj, JPy_Byte_JClass);
    DEFINE_OBJECT_TYPE(JPy_JShortObj, JPy_Short_JClass);
    DEFINE_OBJECT_TYPE(JPy_JIntegerObj, JPy_Integer_JClass);
    DEFINE_OBJECT_TYPE(JPy_JLongObj, JPy_Long_JClass);
    DEFINE_OBJECT_TYPE(JPy_JFloatObj, JPy_Float_JClass);
    DEFINE_OBJECT_TYPE(JPy_JDoubleObj, JPy_Double_JClass);
    // Other objects.
    DEFINE_OBJECT_TYPE(JPy_JString, JPy_String_JClass);

    return 0;
}

void JPy_ClearGlobalVars(void)
{
    // todo - For all global class refs: (*jenv)->DeleteGlobalRef(jenv, classRef)

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

    JPy_Field_JClass = NULL;
    JPy_Field_GetName_MID = NULL;
    JPy_Field_GetModifiers_MID = NULL;
    JPy_Field_GetType_MID = NULL;

    JPy_RuntimeException_JClass = NULL;



    Py_DECREF(JPy_JBoolean);
    Py_DECREF(JPy_JChar);
    Py_DECREF(JPy_JByte);
    Py_DECREF(JPy_JShort);
    Py_DECREF(JPy_JInt);
    Py_DECREF(JPy_JLong);
    Py_DECREF(JPy_JFloat);
    Py_DECREF(JPy_JDouble);
    Py_DECREF(JPy_JVoid);
    Py_DECREF(JPy_JBooleanObj);
    Py_DECREF(JPy_JCharacterObj);
    Py_DECREF(JPy_JByteObj);
    Py_DECREF(JPy_JShortObj);
    Py_DECREF(JPy_JIntegerObj);
    Py_DECREF(JPy_JLongObj);
    Py_DECREF(JPy_JFloatObj);
    Py_DECREF(JPy_JDoubleObj);

    JPy_JBoolean = NULL;
    JPy_JChar = NULL;
    JPy_JByte = NULL;
    JPy_JShort = NULL;
    JPy_JInt = NULL;
    JPy_JLong = NULL;
    JPy_JFloat = NULL;
    JPy_JDouble = NULL;
    JPy_JVoid = NULL;
    JPy_JString = NULL;
    JPy_JBooleanObj = NULL;
    JPy_JCharacterObj = NULL;
    JPy_JByteObj = NULL;
    JPy_JShortObj = NULL;
    JPy_JIntegerObj = NULL;
    JPy_JLongObj = NULL;
    JPy_JFloatObj = NULL;
    JPy_JDoubleObj = NULL;
}


void JPy_HandleJavaException(JNIEnv* jenv)
{
    jthrowable error = (*jenv)->ExceptionOccurred(jenv);
    if (error != NULL) {
        jstring message;

        if (JPy_IsDebug()) {
            (*jenv)->ExceptionDescribe(jenv);
        }

        message = (jstring) (*jenv)->CallObjectMethod(jenv, error, JPy_Object_ToString_MID);
        if (message != NULL) {
            const char* messageChars;

            messageChars = (*jenv)->GetStringUTFChars(jenv, message, NULL);
            if (messageChars != NULL) {
                PyErr_Format(PyExc_RuntimeError, "%s", messageChars);
                (*jenv)->ReleaseStringUTFChars(jenv, message, messageChars);
            } else {
                PyErr_SetString(PyExc_RuntimeError, "Java VM exception occurred, but failed to allocate message text");
            }
            (*jenv)->DeleteLocalRef(jenv, message);
        } else {
            PyErr_SetString(PyExc_RuntimeError, "Java VM exception occurred, no message");
        }

        (*jenv)->DeleteLocalRef(jenv, error);
        (*jenv)->ExceptionClear(jenv);
    }
}

void JPy_HandlePythonException(JNIEnv* jenv)
{
    PyObject* pyType;
    PyObject* pyValue;
    PyObject* pyTraceback;

    PyObject* pyTypeStr;
    PyObject* pyValueStr;
    PyObject* pyTracebackStr;

    PyObject* pyTypeUtf8;
    PyObject* pyValueUtf8;
    PyObject* pyTracebackUtf8;

    char* typeChars;
    char* valueChars;
    char* tracebackChars;
    char* javaMessage;

    jint ret;

    if (PyErr_Occurred() == NULL) {
        return;
    }

    PyErr_Fetch(&pyType, &pyValue, &pyTraceback);
    PyErr_NormalizeException(&pyType, &pyValue, &pyTraceback);

    //printf("JPy_HandlePythonException 1: %p, %p, %p\n", pyType, pyValue, pyTraceback);

    pyTypeStr = pyType != NULL ? PyObject_Str(pyType) : NULL;
    pyValueStr = pyValue != NULL ? PyObject_Str(pyValue) : NULL;
    pyTracebackStr = pyTraceback != NULL ? PyObject_Str(pyTraceback) : NULL;

    //printf("JPy_HandlePythonException 2: %p, %p, %p\n", pyTypeStr, pyValueStr, pyTracebackStr);

    pyTypeUtf8 = pyTypeStr != NULL ? PyUnicode_AsEncodedString(pyTypeStr, "utf-8", "replace") : NULL;
    pyValueUtf8 = pyValueStr != NULL ? PyUnicode_AsEncodedString(pyValueStr, "utf-8", "replace") : NULL;
    pyTracebackUtf8 = pyTracebackStr != NULL ? PyUnicode_AsEncodedString(pyTracebackStr, "utf-8", "replace") : NULL;

    //printf("JPy_HandlePythonException 3: %p, %p, %p\n", pyTypeUtf8, pyValueUtf8, pyTracebackUtf8);

    typeChars = pyTypeUtf8 != NULL ? PyBytes_AsString(pyTypeUtf8) : NULL;
    valueChars = pyValueUtf8 != NULL ? PyBytes_AsString(pyValueUtf8) : NULL;
    tracebackChars = pyTracebackUtf8 != NULL ? PyBytes_AsString(pyTracebackUtf8) : NULL;

    //printf("JPy_HandlePythonException 4: %s, %s, %s\n", typeChars, valueChars, tracebackChars);

    javaMessage = PyMem_New(char,
                            (typeChars != NULL ? strlen(typeChars) : 8)
                           + (valueChars != NULL ? strlen(valueChars) : 8)
                           + (tracebackChars != NULL ? strlen(tracebackChars) : 8) + 80);
    if (javaMessage != NULL) {
        sprintf(javaMessage, "An error occurred in the Python interpreter:\n%s: %s\n%s", typeChars, valueChars, tracebackChars);
        ret = (*jenv)->ThrowNew(jenv, JPy_RuntimeException_JClass, javaMessage);
    } else {
        ret = (*jenv)->ThrowNew(jenv, JPy_RuntimeException_JClass, valueChars);
    }

    //printf("JPy_HandlePythonException 5: %s\n", javaMessage);

    PyMem_Del(javaMessage);

    Py_DECREF(pyType);
    Py_DECREF(pyValue);
    Py_DECREF(pyTraceback);

    Py_DECREF(pyTypeStr);
    Py_DECREF(pyValueStr);
    Py_DECREF(pyTracebackStr);

    Py_DECREF(pyTypeUtf8);
    Py_DECREF(pyValueUtf8);
    Py_DECREF(pyTracebackUtf8);
}
