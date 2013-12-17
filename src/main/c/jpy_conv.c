#include "jpy_module.h"
#include "jpy_conv.h"

/**
 * Copies the UTF, zero-terminated C-string.
 * Caller is responsible for freeing the returned string using PyMem_Del().
 */
char* JPy_CopyUTFString(const char* utfChars)
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
 * Copies the given jchar string used by Java into a wchar_t string used by Python.
 * Caller is responsible for freeing the returned string using PyMem_Del().
 */
wchar_t* JPy_ConvertToWCharString(const jchar* jChars, jint length)
{
    wchar_t* wChars;
    jint i;

    wChars = PyMem_New(wchar_t, length + 1);
    if (wChars == NULL) {
        PyErr_NoMemory();
        return NULL;
    }

    for (i = 0; i < length; i++) {
        wChars[i] = (wchar_t) jChars[i];
    }
    wChars[length] = 0;

    return wChars;
}

/**
 * Copies the given wchar_t string used by Python into a jchar string used by Python.
 * Caller is responsible for freeing the returned string using PyMem_Del().
 */
jchar* JPy_ConvertToJCharString(const wchar_t* wChars, jint length)
{
    jchar* jChars;
    jint i;

    jChars = PyMem_New(jchar, length + 1);
    if (jChars == NULL) {
        PyErr_NoMemory();
        return NULL;
    }

    for (i = 0; i < length; i++) {
        jChars[i] = (jchar) wChars[i];
    }
    jChars[length] = (jchar) 0;

    return jChars;
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
    typeNameCopy = JPy_CopyUTFString(typeName);
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
    const jchar* jChars;
    jint length;

    if (stringRef == NULL) {
        return Py_BuildValue("");
    }

    // todo: handle errors
    length = (*jenv)->GetStringLength(jenv, stringRef);
    if (length == 0) {
        return Py_BuildValue("s", "");
    }

    jChars = (*jenv)->GetStringChars(jenv, stringRef, NULL);
    returnValue = PyUnicode_FromKindAndData(PyUnicode_2BYTE_KIND, jChars, length);
    (*jenv)->ReleaseStringChars(jenv, stringRef, jChars);

    return returnValue;
}

/**
 * Returns a new Java string (a local reference).
 */
int JPy_ConvertPythonToJavaString(JNIEnv* jenv, PyObject* arg, jstring* stringRef)
{
    Py_ssize_t length;
    wchar_t* wChars;

    if (arg == Py_None) {
        *stringRef = NULL;
        return 0;
    }

    wChars = PyUnicode_AsWideCharString(arg, &length);
    if (wChars == NULL) {
        *stringRef = NULL;
        return -1;
    }

    if (sizeof(wchar_t) == sizeof(jchar)) {
        *stringRef = (*jenv)->NewString(jenv, (const jchar*) wChars, length);
    } else {
        jchar* jChars;
        jChars = JPy_ConvertToJCharString(wChars, length);
        if (jChars == NULL) {
            goto error;
        }
        *stringRef = (*jenv)->NewString(jenv, jChars, length);
        PyMem_Del(jChars);
    }
    if (*stringRef == NULL) {
        PyMem_Del(wChars);
        PyErr_NoMemory();
        return -1;
    }

error:
    PyMem_Del(wChars);

    return 0;
}


int JPy_ConvertPythonToJavaObject(JNIEnv* jenv, PyObject* pyObj, jobject* objectRef)
{
    // todo: implement JPy_ConvertPythonToJavaObject
    printf("JPy_ConvertPythonToJavaObject: pyObj=%p (NOT IMPLEMENTED!)\n", pyObj);
    PyErr_SetString(PyExc_RuntimeError, "NOT IMPLEMENTED");
    return -1;
}
