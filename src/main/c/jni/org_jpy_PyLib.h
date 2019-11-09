/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class org_jpy_PyLib */

#ifndef _Included_org_jpy_PyLib
#define _Included_org_jpy_PyLib
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     org_jpy_PyLib
 * Method:    isPythonRunning
 * Signature: ()Z
 */
JNIEXPORT jboolean JNICALL Java_org_jpy_PyLib_isPythonRunning
  (JNIEnv *, jclass);

/*
 * Class:     org_jpy_PyLib
 * Method:    setPythonHome
 * Signature: (Ljava/lang/String;)Z
 */
JNIEXPORT jint JNICALL Java_org_jpy_PyLib_setPythonHome
  (JNIEnv* jenv, jclass jLibClass, jstring jPythonHome);

/*
 * Class:     org_jpy_PyLib
 * Method:    startPython0
 * Signature: ([Ljava/lang/String;)Z
 */
JNIEXPORT jboolean JNICALL Java_org_jpy_PyLib_startPython0
  (JNIEnv *, jclass, jobjectArray);

/*
 * Class:     org_jpy_PyLib
 * Method:    getPythonVersion
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_org_jpy_PyLib_getPythonVersion
  (JNIEnv *, jclass);

/*
 * Class:     org_jpy_PyLib
 * Method:    stopPython0
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_org_jpy_PyLib_stopPython0
  (JNIEnv *, jclass);

/*
 * Class:     org_jpy_PyLib
 * Method:    execScript
 * Signature: (Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_org_jpy_PyLib_execScript
  (JNIEnv *, jclass, jstring);

/*
 * Class:     org_jpy_PyLib
 * Method:    executeCode
 * Signature: (Ljava/lang/String;ILjava/lang/Object;Ljava/lang/Object;)J
 */
JNIEXPORT jlong JNICALL Java_org_jpy_PyLib_executeCode
  (JNIEnv *, jclass, jstring, jint, jobject, jobject);

/*
 * Class:     org_jpy_PyLib
 * Method:    executeScript
 * Signature: (Ljava/lang/String;ILjava/lang/Object;Ljava/lang/Object;)J
 */
JNIEXPORT jlong JNICALL Java_org_jpy_PyLib_executeScript
  (JNIEnv *, jclass, jstring, jint, jobject, jobject);

/*
 * Class:     org_jpy_PyLib
 * Method:    getMainGlobals
 * Signature: ()Lorg/jpy/PyObject;
 */
JNIEXPORT jobject JNICALL Java_org_jpy_PyLib_getMainGlobals
  (JNIEnv *, jclass);

/*
 * Class:     org_jpy_PyLib
 * Method:    copyDict
 * Signature: (J)Lorg/jpy/PyObject;
 */
JNIEXPORT jobject JNICALL Java_org_jpy_PyLib_copyDict
  (JNIEnv *, jclass, jlong);

/*
 * Class:     org_jpy_PyLib
 * Method:    incRef
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_org_jpy_PyLib_incRef
  (JNIEnv *, jclass, jlong);

/*
 * Class:     org_jpy_PyLib
 * Method:    decRef
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_org_jpy_PyLib_decRef
  (JNIEnv *, jclass, jlong);

/*
 * Class:     org_jpy_PyLib
 * Method:    getIntValue
 * Signature: (J)I
 */
JNIEXPORT jint JNICALL Java_org_jpy_PyLib_getIntValue
  (JNIEnv *, jclass, jlong);

/*
 * Class:     org_jpy_PyLib
 * Method:    getLongValue
 * Signature: (J)J
 */
JNIEXPORT jlong JNICALL Java_org_jpy_PyLib_getLongValue
  (JNIEnv *, jclass, jlong);

/*
 * Class:     org_jpy_PyLib
 * Method:    getBooleanValue
 * Signature: (J)Z
 */
JNIEXPORT jboolean JNICALL Java_org_jpy_PyLib_getBooleanValue
  (JNIEnv *, jclass, jlong);

/*
 * Class:     org_jpy_PyLib
 * Method:    getDoubleValue
 * Signature: (J)D
 */
JNIEXPORT jdouble JNICALL Java_org_jpy_PyLib_getDoubleValue
  (JNIEnv *, jclass, jlong);

/*
 * Class:     org_jpy_PyLib
 * Method:    getStringValue
 * Signature: (J)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_org_jpy_PyLib_getStringValue
  (JNIEnv *, jclass, jlong);

/*
 * Class:     org_jpy_PyLib
 * Method:    getObjectValue
 * Signature: (J)Ljava/lang/Object;
 */
JNIEXPORT jobject JNICALL Java_org_jpy_PyLib_getObjectValue
  (JNIEnv *, jclass, jlong);

/*
 * Class:     org_jpy_PyLib
 * Method:    isConvertible
 * Signature: (J)Z
 */
JNIEXPORT jboolean JNICALL Java_org_jpy_PyLib_isConvertible
  (JNIEnv *, jclass, jlong);

/*
 * Class:     org_jpy_PyLib
 * Method:    pyNoneCheck
 * Signature: (J)Z
 */
JNIEXPORT jboolean JNICALL Java_org_jpy_PyLib_pyNoneCheck
  (JNIEnv *, jclass, jlong);

/*
 * Class:     org_jpy_PyLib
 * Method:    pyDictCheck
 * Signature: (J)Z
 */
JNIEXPORT jboolean JNICALL Java_org_jpy_PyLib_pyDictCheck
  (JNIEnv *, jclass, jlong);

/*
 * Class:     org_jpy_PyLib
 * Method:    pyListCheck
 * Signature: (J)Z
 */
JNIEXPORT jboolean JNICALL Java_org_jpy_PyLib_pyListCheck
  (JNIEnv *, jclass, jlong);

/*
 * Class:     org_jpy_PyLib
 * Method:    pyBoolCheck
 * Signature: (J)Z
 */
JNIEXPORT jboolean JNICALL Java_org_jpy_PyLib_pyBoolCheck
  (JNIEnv *, jclass, jlong);

/*
 * Class:     org_jpy_PyLib
 * Method:    pyIntCheck
 * Signature: (J)Z
 */
JNIEXPORT jboolean JNICALL Java_org_jpy_PyLib_pyIntCheck
  (JNIEnv *, jclass, jlong);

/*
 * Class:     org_jpy_PyLib
 * Method:    pyLongCheck
 * Signature: (J)Z
 */
JNIEXPORT jboolean JNICALL Java_org_jpy_PyLib_pyLongCheck
  (JNIEnv *, jclass, jlong);

/*
 * Class:     org_jpy_PyLib
 * Method:    pyFloatCheck
 * Signature: (J)Z
 */
JNIEXPORT jboolean JNICALL Java_org_jpy_PyLib_pyFloatCheck
  (JNIEnv *, jclass, jlong);

/*
 * Class:     org_jpy_PyLib
 * Method:    pyStringCheck
 * Signature: (J)Z
 */
JNIEXPORT jboolean JNICALL Java_org_jpy_PyLib_pyStringCheck
  (JNIEnv *, jclass, jlong);

/*
 * Class:     org_jpy_PyLib
 * Method:    pyCallableCheck
 * Signature: (J)Z
 */
JNIEXPORT jboolean JNICALL Java_org_jpy_PyLib_pyCallableCheck
  (JNIEnv *, jclass, jlong);

/*
 * Class:     org_jpy_PyLib
 * Method:    getType
 * Signature: (J)J
 */
JNIEXPORT jlong JNICALL Java_org_jpy_PyLib_getType
  (JNIEnv *, jclass, jlong);

/*
 * Class:     org_jpy_PyLib
 * Method:    str
 * Signature: (J)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_org_jpy_PyLib_str
  (JNIEnv *, jclass, jlong);

/*
 * Class:     org_jpy_PyLib
 * Method:    repr
 * Signature: (J)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_org_jpy_PyLib_repr
  (JNIEnv *, jclass, jlong);

/*
 * Class:     org_jpy_PyLib
 * Method:    hash
 * Signature: (J)J
 */
JNIEXPORT jlong JNICALL Java_org_jpy_PyLib_hash
  (JNIEnv *, jclass, jlong);

/*
 * Class:     org_jpy_PyLib
 * Method:    eq
 * Signature: (JLjava/lang/Object;)Z
 */
JNIEXPORT jboolean JNICALL Java_org_jpy_PyLib_eq
  (JNIEnv *, jclass, jlong, jobject);

/*
 * Class:     org_jpy_PyLib
 * Method:    newDict
 * Signature: ()Lorg/jpy/PyObject;
 */
JNIEXPORT jobject JNICALL Java_org_jpy_PyLib_newDict
  (JNIEnv *, jclass);

/*
 * Class:     org_jpy_PyLib
 * Method:    getObjectArrayValue
 * Signature: (JLjava/lang/Class;)[Ljava/lang/Object;
 */
JNIEXPORT jobjectArray JNICALL Java_org_jpy_PyLib_getObjectArrayValue
  (JNIEnv *, jclass, jlong, jclass);

/*
 * Class:     org_jpy_PyLib
 * Method:    importModule
 * Signature: (Ljava/lang/String;)J
 */
JNIEXPORT jlong JNICALL Java_org_jpy_PyLib_importModule
  (JNIEnv *, jclass, jstring);

/*
 * Class:     org_jpy_PyLib
 * Method:    getAttributeObject
 * Signature: (JLjava/lang/String;)J
 */
JNIEXPORT jlong JNICALL Java_org_jpy_PyLib_getAttributeObject
  (JNIEnv *, jclass, jlong, jstring);

/*
 * Class:     org_jpy_PyLib
 * Method:    getAttributeValue
 * Signature: (JLjava/lang/String;Ljava/lang/Class;)Ljava/lang/Object;
 */
JNIEXPORT jobject JNICALL Java_org_jpy_PyLib_getAttributeValue
  (JNIEnv *, jclass, jlong, jstring, jclass);

/*
 * Class:     org_jpy_PyLib
 * Method:    setAttributeValue
 * Signature: (JLjava/lang/String;Ljava/lang/Object;Ljava/lang/Class;)V
 */
JNIEXPORT void JNICALL Java_org_jpy_PyLib_setAttributeValue
  (JNIEnv *, jclass, jlong, jstring, jobject, jclass);

/*
 * Class:     org_jpy_PyLib
 * Method:    delAttribute
 * Signature: (JLjava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_org_jpy_PyLib_delAttribute
  (JNIEnv *, jclass, jlong, jstring);

/*
 * Class:     org_jpy_PyLib
 * Method:    hasAttribute
 * Signature: (JLjava/lang/String;)Z
 */
JNIEXPORT jboolean JNICALL Java_org_jpy_PyLib_hasAttribute
  (JNIEnv *, jclass, jlong, jstring);

/*
 * Class:     org_jpy_PyLib
 * Method:    callAndReturnObject
 * Signature: (JZLjava/lang/String;I[Ljava/lang/Object;[Ljava/lang/Class;)J
 */
JNIEXPORT jlong JNICALL Java_org_jpy_PyLib_callAndReturnObject
  (JNIEnv *, jclass, jlong, jboolean, jstring, jint, jobjectArray, jobjectArray);

/*
 * Class:     org_jpy_PyLib
 * Method:    callAndReturnValue
 * Signature: (JZLjava/lang/String;I[Ljava/lang/Object;[Ljava/lang/Class;Ljava/lang/Class;)Ljava/lang/Object;
 */
JNIEXPORT jobject JNICALL Java_org_jpy_PyLib_callAndReturnValue
  (JNIEnv *, jclass, jlong, jboolean, jstring, jint, jobjectArray, jobjectArray, jclass);

#ifdef __cplusplus
}
#endif
#endif
