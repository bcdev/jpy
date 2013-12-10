#ifndef JPY_MODULE_H
#define JPY_MODULE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <Python.h>
#include <structmember.h>
#include <jni.h>

extern PyObject* JPy_Module;
extern PyObject* JPy_Exception;
extern PyObject* JPy_Types;

#define JPy_JINIT_ATTR_NAME "__jinit__"

JNIEnv* JPy_GetJNIEnv();
int JPy_IsDebug();

#define JPY_GET_JENV(JENV, RET_VALUE) \
    if ((JENV = JPy_GetJNIEnv()) == NULL) { \
        PyErr_SetString(PyExc_RuntimeError, "Java VM not available (use jpy.create_jvm() to create one)"); \
        return (RET_VALUE); \
    } else { \
    }



extern PyTypeObject* JPy_JBoolean;
extern PyTypeObject* JPy_JByte;
extern PyTypeObject* JPy_JShort;
extern PyTypeObject* JPy_JInt;
extern PyTypeObject* JPy_JLong;
extern PyTypeObject* JPy_JFloat;
extern PyTypeObject* JPy_JDouble;
extern PyTypeObject* JPy_JChar;
extern PyTypeObject* JPy_JVoid;
extern PyTypeObject* JPy_JString;



// java.lang.Comparable
extern jclass JPy_Comparable_JClass;
// java.lang.Object
extern jclass JPy_Object_JClass;
extern jmethodID JPy_Object_ToString_MID;
extern jmethodID JPy_Object_HashCode_MID;
extern jmethodID JPy_Object_Equals_MID;
// java.lang.Class
extern jclass JPy_Class_JClass;
extern jmethodID JPy_Class_GetName_MID;
extern jmethodID JPy_Class_GetDeclaredConstructors_MID;
extern jmethodID JPy_Class_GetDeclaredMethods_MID;
extern jmethodID JPy_Class_GetComponentType_MID;
extern jmethodID JPy_Class_IsPrimitive_MID;
// java.lang.reflect.Constructor
extern jclass JPy_Constructor_JClass;
extern jmethodID JPy_Constructor_GetModifiers_MID;
extern jmethodID JPy_Constructor_GetParameterTypes_MID;
// java.lang.reflect.Method
extern jclass JPy_Method_JClass;
extern jmethodID JPy_Method_GetName_MID;
extern jmethodID JPy_Method_GetModifiers_MID;
extern jmethodID JPy_Method_GetParameterTypes_MID;
extern jmethodID JPy_Method_GetReturnType_MID;


////////////////////////////////////////////////////////////////////
// Helpers


/**
 * Copies the UTF name.
 * Caller is responsible for freeing the returned string using Py_Del().
 */
char* JPy_CopyUTF(const char* utfChars);

/**
 * Gets the UTF name of thie given class.
 * Caller is responsible for freeing the returned string using Py_Del().
 */
char* JPy_AllocTypeNameUTF(JNIEnv* jenv, jclass classRef);

/**
 * Gets a string object representing the name of the given class.
 * Returns a new reference.
 */
PyObject* JPy_GetTypeNameString(JNIEnv* jenv, jclass classRef);

PyObject* JPy_ConvertJavaToPythonString(JNIEnv* jenv, jstring stringRef);

PyObject* JPy_ConvertJavaToStringToPythonString(JNIEnv* jenv, jobject objectRef);

int JPy_ConvertPythonToJavaString(JNIEnv* jenv, PyObject* arg, jstring* stringRef);

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif /* !JPY_MODULE_H */