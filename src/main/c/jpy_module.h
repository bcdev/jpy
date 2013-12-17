#ifndef JPY_MODULE_H
#define JPY_MODULE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <Python.h>
#include <structmember.h>
#include <jni.h>

extern PyObject* JPy_Module;
extern PyObject* JPy_Types;
extern PyObject* JPy_Type_Callbacks;
extern PyObject* JException_Type;

#define JPy_JINIT_ATTR_NAME "__jinit__"

/**
 * Gets the current JNI environment pointer.
 * Returns NULL, if the JVM is down.
 *
 * General jpy design guideline: Use the JPy_GetJNIEnv function only in entry points from Python calls into C.
 * Add a JNIEnv* as first parameter to all functions that require it.
 */
JNIEnv* JPy_GetJNIEnv(void);

/**
 * Gets the current JNI environment pointer JENV. If this is NULL, it returns the given RET_VALUE.
 * Warning: This method may immediately return, so make sure there will be no memory leaks in this case.
 *
 * General jpy design guideline: Use the JPy_GET_JNI_ENV_OR_RETURN macro only in entry points from Python calls into C.
 * Add a JNIEnv* as first parameter to all functions that require it.
 */
#define JPy_GET_JNI_ENV_OR_RETURN(JENV, RET_VALUE) \
    if ((JENV = JPy_GetJNIEnv()) == NULL) { \
        PyErr_SetString(PyExc_RuntimeError, "Java VM not available (use jpy.create_jvm() to create one)"); \
        return (RET_VALUE); \
    } else { \
    }

/**
 * Checks if we are in debug mode.
 */
jboolean JPy_IsDebug(void);

/**
 * Sets if we are in debug mode.
 */
void JPy_SetDebug(jboolean debug);

struct JPy_JType;

extern struct JPy_JType* JPy_JBoolean;
extern struct JPy_JType* JPy_JByte;
extern struct JPy_JType* JPy_JShort;
extern struct JPy_JType* JPy_JInt;
extern struct JPy_JType* JPy_JLong;
extern struct JPy_JType* JPy_JFloat;
extern struct JPy_JType* JPy_JDouble;
extern struct JPy_JType* JPy_JChar;
extern struct JPy_JType* JPy_JVoid;
extern struct JPy_JType* JPy_JString;


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
extern jmethodID JPy_Class_GetDeclaredFields_MID;
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
// java.lang.reflect.Field
extern jclass JPy_Field_JClass;
extern jmethodID JPy_Field_GetName_MID;
extern jmethodID JPy_Field_GetModifiers_MID;
extern jmethodID JPy_Field_GetType_MID;


#ifdef __cplusplus
} /* extern "C" */
#endif
#endif /* !JPY_MODULE_H */