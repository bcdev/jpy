#include "jpy_module.h"
#include "jpy_jtype.h"
#include "jpy_jmethod.h"
#include "jpy_jobj.h"
#include "jpy_carray.h"
#include <stdlib.h>
#include <string.h>


PyObject* JPy_create_jvm(PyObject* self, PyObject* args, PyObject* kwds);
PyObject* JPy_destroy_jvm(PyObject* self, PyObject* args);
PyObject* JPy_get_jtype(PyObject* self, PyObject* args, PyObject* kwds);
PyObject* JPy_cast(PyObject* self, PyObject* args);

static PyMethodDef JPy_Functions[] = {

    {"create_jvm",  (PyCFunction) JPy_create_jvm, METH_VARARGS|METH_KEYWORDS,
                    "create_jvm(options, debug=False) - create the Java VM from the given list of options."},

    {"destroy_jvm", JPy_destroy_jvm, METH_VARARGS,
                    "destroy_jvm() - destroys the current Java VM."},

    {"get_jtype",   (PyCFunction) JPy_get_jtype, METH_VARARGS|METH_KEYWORDS,
                    "get_jtype(name, resolve=True) - gets the Java type with the given name. Loads it if not already done. Resolves its methods."},

    {"cast",        JPy_cast, METH_VARARGS,
                    "cast(jobj, class_name) - casts jobj to the Java class given by class_name. Returns None if the cast is not possible"},

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
PyObject* JPy_Exception = NULL;
PyObject* JPy_Types = NULL;

typedef struct {
    // A global reference to a Java VM.
    JavaVM* jvm;
    // The current JNI interface.
    JNIEnv* jenv;
    // Used for switching debug prints in this module
    int DEBUG;
} JPy_JVM;

static JPy_JVM JVM = {NULL, NULL, 0};

#define JPY_JNI_VERSION JNI_VERSION_1_6



PyTypeObject* JPy_JBoolean = NULL;
PyTypeObject* JPy_JByte = NULL;
PyTypeObject* JPy_JShort = NULL;
PyTypeObject* JPy_JInt = NULL;
PyTypeObject* JPy_JLong = NULL;
PyTypeObject* JPy_JFloat = NULL;
PyTypeObject* JPy_JDouble = NULL;
PyTypeObject* JPy_JChar = NULL;
PyTypeObject* JPy_JVoid = NULL;
PyTypeObject* JPy_JString = NULL;


// Global VM Information (todo: put this into JPy_JVM structure)
// {{{

jclass JPy_Comparable_JClass = NULL;

jclass JPy_Object_JClass = NULL;
jmethodID JPy_Object_ToString_MID = NULL;
jmethodID JPy_Object_HashCode_MID = NULL;
jmethodID JPy_Object_Equals_MID = NULL;

jclass JPy_Class_JClass = NULL;
jmethodID JPy_Class_GetName_MID = NULL;
jmethodID JPy_Class_GetDeclaredConstructors_MID = NULL;
jmethodID JPy_Class_GetDeclaredMethods_MID = NULL;
jmethodID JPy_Class_GetComponentType_MID = NULL;
jmethodID JPy_Class_IsPrimitive_MID = NULL;

jclass JPy_Constructor_JClass = NULL;
jmethodID JPy_Constructor_GetModifiers_MID = NULL;
jmethodID JPy_Constructor_GetParameterTypes_MID = NULL;

jclass JPy_Method_JClass = NULL;
jmethodID JPy_Method_GetName_MID = NULL;
jmethodID JPy_Method_GetReturnType_MID = NULL;
jmethodID JPy_Method_GetParameterTypes_MID = NULL;
jmethodID JPy_Method_GetModifiers_MID = NULL;
// }}}

int JPy_InitGlobalVars(JNIEnv* jenv);


/**
 * Called if the JVM loads this module.
 * Will only called if this module's code is linked into a shared library and loaded by a Java VM.
 */
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* jvm, void* reserved)
{
    printf("jpy: JNI_OnLoad() called\n");
    JVM.jvm = jvm;
    (*jvm)->GetEnv(jvm, (void**) &JVM.jenv, JPY_JNI_VERSION);

    // todo: initialise Python interpreter

    return JPY_JNI_VERSION;
}

/**
 * Called if the JVM unloads this module.
 * Will only called if this module's code is linked into a shared library and loaded by a Java VM.
 */
JNIEXPORT void JNICALL JNI_OnUnload(JavaVM* jvm, void* reserved)
{
    printf("jpy: JNI_OnUnload() called\n");
    JVM.jvm = NULL;
    JVM.jenv = NULL;

    // todo: shutdown Python interpreter
}

/**
 * Called by the Python interpreter's import machinery, e.g. using 'import jpy'.
 */
PyMODINIT_FUNC PyInit_jpy(void)
{
    //printf("PyInit_jpy: enter\n");

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

    JPy_Exception = PyErr_NewException("jpy.Exception", NULL, NULL);
    Py_INCREF(JPy_Exception);
    PyModule_AddObject(JPy_Module, "jexception", JPy_Exception);

    /////////////////////////////////////////////////////////////////////////

    JPy_Types = PyDict_New();
    Py_INCREF(JPy_Types);
    PyModule_AddObject(JPy_Module, "jtypes", JPy_Types);

    /////////////////////////////////////////////////////////////////////////

    //printf("PyInit_jpy: exit\n");

    return JPy_Module;
}

JNIEnv* JPy_GetJNIEnv()
{
    // Currently JPy is only single threaded.
    // To make it multi-threaded we must use the following code:
    // (*JVM.jvm)->AttachCurrentThread(JVM.jvm, &JVM.jenv, NULL);
    return JVM.jenv;
}

int JPy_IsDebug()
{
    return JVM.DEBUG;
}

PyObject* JPy_create_jvm(PyObject* self, PyObject* args, PyObject* kwds)
{
    static char* keywords[] = {"options", "debug", NULL};
    PyObject*   options = NULL;
    Py_ssize_t  optionCount;
    PyObject*   option;
    JavaVMOption* jvmOptions;
    JavaVMInitArgs jvmInitArgs;
    Py_ssize_t  i;
    jint        res;

    if (JVM.jvm != NULL) {
        PyErr_SetString(PyExc_RuntimeError, "only a single Java VM can be instantiated (use jvm.destroy_jvm() first)");
        return NULL;
    }

    JVM.DEBUG = JNI_FALSE;
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O|p", keywords, &options, &JVM.DEBUG)) {
        return NULL;
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

    if (JPy_IsDebug()) printf("JPy_create_jvm: res=%d, JVM.jvm=%p, JVM.jenv=%p, JVM.DEBUG=%d\n", res, JVM.jvm, JVM.jenv, JVM.DEBUG);

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

    if (JVM.jvm != NULL) {
        (*JVM.jvm)->DestroyJavaVM(JVM.jvm);
    }

    JVM.jvm = NULL;
    JVM.jenv = NULL;

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

    return Py_BuildValue("");
}

PyObject* JPy_get_jtype(PyObject* self, PyObject* args, PyObject* kwds)
{
    static char* keywords[] = {"name", "resolve", NULL};
    const char* className;
    int resolve;

    resolve = JNI_TRUE;
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "s|p", keywords, &className, &resolve)) {
        return NULL;
    }

    return (PyObject*) JType_GetTypeForName(className, (jboolean) resolve);
}

PyObject* JPy_cast(PyObject* self, PyObject* args)
{
    JNIEnv* jenv;
    PyObject* obj;
    PyObject* objType;
    jboolean inst;

    JPY_GET_JENV(jenv, NULL)

    if (!PyArg_ParseTuple(args, "OO", &obj, &objType)) {
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


PyTypeObject* JPy_GetNonObjectJType(JNIEnv* jenv, const char* wrapperClassName)
{
    jclass wrapperClassRef;
    jclass primClassRef;
    jfieldID fid;
    PyTypeObject* type;

    wrapperClassRef = (*jenv)->FindClass(jenv, wrapperClassName);
    fid = (*jenv)->GetStaticFieldID(jenv, wrapperClassRef, "TYPE", "Ljava/lang/Class;");
    primClassRef = (*jenv)->GetStaticObjectField(jenv, wrapperClassRef, fid);
    type = JType_GetType(primClassRef, JNI_FALSE);
    ((JPy_JType*) type)->isResolved = JNI_TRUE; // Primitive types are always resolved.
    Py_INCREF(type);

    return type;
}


int JPy_InitGlobalVars(JNIEnv* jenv)
{
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
    JPy_Class_GetComponentType_MID = (*jenv)->GetMethodID(jenv, JPy_Class_JClass, "getComponentType", "()Ljava/lang/Class;");
    JPy_Class_IsPrimitive_MID = (*jenv)->GetMethodID(jenv, JPy_Class_JClass, "isPrimitive", "()Z");

    JPy_Constructor_JClass = (*jenv)->FindClass(jenv, "java/lang/reflect/Constructor");
    JPy_Constructor_GetModifiers_MID = (*jenv)->GetMethodID(jenv, JPy_Constructor_JClass, "getModifiers", "()I");
    JPy_Constructor_GetParameterTypes_MID = (*jenv)->GetMethodID(jenv, JPy_Constructor_JClass, "getParameterTypes", "()[Ljava/lang/Class;");

    JPy_Method_JClass = (*jenv)->FindClass(jenv, "java/lang/reflect/Method");
    JPy_Method_GetName_MID = (*jenv)->GetMethodID(jenv, JPy_Method_JClass, "getName", "()Ljava/lang/String;");
    JPy_Method_GetModifiers_MID = (*jenv)->GetMethodID(jenv, JPy_Method_JClass, "getModifiers", "()I");
    JPy_Method_GetParameterTypes_MID = (*jenv)->GetMethodID(jenv, JPy_Method_JClass, "getParameterTypes", "()[Ljava/lang/Class;");
    JPy_Method_GetReturnType_MID = (*jenv)->GetMethodID(jenv, JPy_Method_JClass, "getReturnType", "()Ljava/lang/Class;");

    JPy_JBoolean = JPy_GetNonObjectJType(jenv, "java/lang/Boolean");
    JPy_JByte = JPy_GetNonObjectJType(jenv, "java/lang/Byte");
    JPy_JShort = JPy_GetNonObjectJType(jenv, "java/lang/Short");
    JPy_JInt = JPy_GetNonObjectJType(jenv, "java/lang/Integer");
    JPy_JLong = JPy_GetNonObjectJType(jenv, "java/lang/Long");
    JPy_JFloat = JPy_GetNonObjectJType(jenv, "java/lang/Float");
    JPy_JDouble = JPy_GetNonObjectJType(jenv, "java/lang/Double");
    JPy_JChar = JPy_GetNonObjectJType(jenv, "java/lang/Character");
    JPy_JVoid = JPy_GetNonObjectJType(jenv, "java/lang/Void");

    {
        jclass c = (*jenv)->FindClass(jenv, "java/lang/String");
        JPy_JString = JType_GetType(c, JNI_FALSE);
    }

    return 0;
}

/**
 * Copies the UTF name.
 * Caller is responsible for freeing the returned string using Py_Del().
 */
char* JPy_CopyUTF(const char* utfChars)
{
    char* utfCharsCopy;

    utfCharsCopy = PyMem_New(char, strlen(utfChars) + 1);
    if (utfCharsCopy == NULL) {
        PyErr_NoMemory();
        return NULL;
    }

    strcpy(utfCharsCopy, utfChars);
    return utfCharsCopy;
}

/**
 * Gets the UTF name of thie given class.
 * Caller is responsible for freeing the returned string using Py_Del().
 */
char* JPy_AllocTypeNameUTF(JNIEnv* jenv, jclass classRef)
{
    jstring typeNameStr;
    const char* typeName;
    char* typeNameCopy;

    // todo: handle errors
    typeNameStr = (*jenv)->CallObjectMethod(jenv, classRef, JPy_Class_GetName_MID);
    typeName = (*jenv)->GetStringUTFChars(jenv, typeNameStr, NULL);
    typeNameCopy = JPy_CopyUTF(typeName);
    (*jenv)->ReleaseStringUTFChars(jenv, classRef, typeName);

    return typeNameCopy;
}

/**
 * Gets a string object representing the name of the given class.
 * Returns a new reference.
 */
PyObject* JPy_GetTypeNameString(JNIEnv* jenv, jclass classRef)
{
    PyObject* typeString;
    jclass typeNameObj;
    const char* typeName;

    // todo: handle errors
    typeNameObj = (*jenv)->CallObjectMethod(jenv, classRef, JPy_Class_GetName_MID);
    typeName = (*jenv)->GetStringUTFChars(jenv, typeNameObj, NULL);
    typeString = Py_BuildValue("s", typeName);
    (*jenv)->ReleaseStringUTFChars(jenv, typeNameObj, typeName);

    return typeString;
}

PyObject* JPy_ConvertJavaToStringToPythonString(JNIEnv* jenv, jobject objectRef)
{
    jstring stringRef;
    PyObject* returnValue;

    if (objectRef == NULL) {
        return Py_BuildValue("");
    }

    // todo: handle errors
    stringRef = (*jenv)->CallObjectMethod(jenv, objectRef, JPy_Object_ToString_MID);
    returnValue = JPy_ConvertJavaToPythonString(jenv, stringRef);
    (*jenv)->DeleteLocalRef(jenv, stringRef);

    return returnValue;
}

PyObject* JPy_ConvertJavaToPythonString(JNIEnv* jenv, jstring stringRef)
{
    PyObject* returnValue;
    jint length;

    if (stringRef == NULL) {
        return Py_BuildValue("");
    }

    // todo: handle errors
    length = (*jenv)->GetStringLength(jenv, stringRef);
    if (length > 0) {
        const jchar* chars;
        chars = (*jenv)->GetStringChars(jenv, stringRef, NULL);
        returnValue = PyUnicode_FromWideChar(chars, length);
        (*jenv)->ReleaseStringChars(jenv, stringRef, chars);
    } else {
        returnValue = Py_BuildValue("s", "");
    }

    return returnValue;
}

/**
 * Returns a new Java string (a local reference).
 */
int JPy_ConvertPythonToJavaString(JNIEnv* jenv, PyObject* arg, jstring* stringRef)
{
    Py_ssize_t length;
    wchar_t* chars;

    if (arg == Py_None) {
        *stringRef = NULL;
        return 0;
    }

    chars = PyUnicode_AsWideCharString(arg, &length);
    if (chars == NULL) {
        return -1;
    }

    *stringRef = (*jenv)->NewString(jenv, chars, length);
    if (*stringRef == NULL) {
        PyMem_Del(chars);
        PyErr_NoMemory();
        return -1;
    }

    PyMem_Del(chars);

    return 0;
}
