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


#define JPy_DEBUG_PRINTF if (JPy_IsDebug()) printf


/**
 * Fetches the last Java exception occurred and raises a new Python exception.
 */
void JPy_HandleJavaException(JNIEnv* jenv);


#define JPy_ON_JAVA_EXCEPTION_GOTO(LABEL) \
    if ((*jenv)->ExceptionCheck(jenv)) { \
        JPy_HandleJavaException(jenv); \
        goto LABEL; \
    }

#define JPy_ON_JAVA_EXCEPTION_RETURN(VALUE) \
    if ((*jenv)->ExceptionCheck(jenv)) { \
        JPy_HandleJavaException(jenv); \
        return VALUE; \
    }


/**
 * Fetches the last Python exception occurred and raises a new Java exception.
 */
void JPy_HandlePythonException();


#define JPy_ON_PYTHON_EXCEPTION_GOTO(LABEL) \
    if (PyErr_Occurred() != NULL) { \
        JPy_HandlePythonException(jenv); \
        goto LABEL; \
    }

#define JPy_ON_PYTHON_EXCEPTION_RETURN(VALUE) \
    if (PyErr_Occurred() != NULL) { \
        JPy_HandlePythonException(jenv); \
        return VALUE; \
    }


struct JPy_JType;

extern struct JPy_JType* JPy_JBoolean;
extern struct JPy_JType* JPy_JChar;
extern struct JPy_JType* JPy_JByte;
extern struct JPy_JType* JPy_JShort;
extern struct JPy_JType* JPy_JInt;
extern struct JPy_JType* JPy_JLong;
extern struct JPy_JType* JPy_JFloat;
extern struct JPy_JType* JPy_JDouble;
extern struct JPy_JType* JPy_JVoid;
extern struct JPy_JType* JPy_JBooleanObj;
extern struct JPy_JType* JPy_JCharacterObj;
extern struct JPy_JType* JPy_JByteObj;
extern struct JPy_JType* JPy_JShortObj;
extern struct JPy_JType* JPy_JIntegerObj;
extern struct JPy_JType* JPy_JLongObj;
extern struct JPy_JType* JPy_JFloatObj;
extern struct JPy_JType* JPy_JDoubleObj;
extern struct JPy_JType* JPy_JObject;
extern struct JPy_JType* JPy_JString;

// java.lang.Comparable
extern jclass JPy_Comparable_JClass;
extern jmethodID JPy_Comparable_CompareTo_MID;
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


extern jclass JPy_RuntimeException_JClass;


extern jclass JPy_Boolean_JClass;
extern jmethodID JPy_Boolean_Init_MID;
extern jmethodID JPy_Boolean_BooleanValue_MID;

extern jclass JPy_Character_JClass;
extern jmethodID JPy_Character_Init_MID;
extern jmethodID JPy_Character_CharValue_MID;

extern jclass JPy_Byte_JClass;
extern jmethodID JPy_Byte_Init_MID;

extern jclass JPy_Short_JClass;
extern jmethodID JPy_Short_Init_MID;

extern jclass JPy_Integer_JClass;
extern jmethodID JPy_Integer_Init_MID;

extern jclass JPy_Long_JClass;
extern jmethodID JPy_Long_Init_MID;

extern jclass JPy_Float_JClass;
extern jmethodID JPy_Float_Init_MID;

extern jclass JPy_Double_JClass;
extern jmethodID JPy_Double_Init_MID;

extern jclass JPy_Number_JClass;
extern jmethodID JPy_Number_IntValue_MID;
extern jmethodID JPy_Number_LongValue_MID;
extern jmethodID JPy_Number_DoubleValue_MID;

extern jclass JPy_String_JClass;
extern jclass JPy_Void_JClass;


#ifdef __cplusplus
} /* extern "C" */
#endif
#endif /* !JPY_MODULE_H */