#ifndef JPY_CONV_H
#define JPY_CONV_H

#ifdef __cplusplus
extern "C" {
#endif


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

/**
 * Calls the toString() method on the given Java object and returns the result as Python unicode string.
 */
PyObject* JPy_ConvertJavaToStringToPythonString(JNIEnv* jenv, jobject objectRef);

int JPy_ConvertPythonToJavaString(JNIEnv* jenv, PyObject* arg, jstring* stringRef);

int JPy_ConvertPythonToJavaObject(JNIEnv* jenv, PyObject* pyObj, jobject* objectRef);


#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* !JPY_CONV_H */