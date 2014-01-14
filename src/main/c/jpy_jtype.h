#ifndef JPY_JTYPE_H
#define JPY_JTYPE_H

#ifdef __cplusplus
extern "C" {
#endif



/**
 * The Python type 'JType' representing a Java type.
 */
typedef struct JPy_JType
{
    // Since this is a type object, inherit everything from from PyTypeObject.
    // It is absolutely essential that this is the first struct member!
    // typeObj.tp_name is this type's fully qualified Java name (same as 'javaName field').
    PyTypeObject typeObj;
    // The Java type name.
    char* javaName;
    // The JNI class reference (global reference).
    jclass classRef;
    // The super type of this type. This will be NULL for primitive types, 'void' and 'java.lang.Object'.
    struct JPy_JType* superType;
    // If component type of this type if this type is an array, NULL otherwise.
    struct JPy_JType* componentType;
    // If TRUE, 'classRef' refers to a primitive type (or 'void').
    char isPrimitive;
    // If TRUE, the type is currently being resolved.
    char isResolving;
    // If TRUE, all the class constructors and methods have already been resolved.
    char isResolved;
}
JPy_JType;

/**
 * The 'JType' singleton.
 */
extern PyTypeObject JType_Type;

typedef int (*JPy_DisposeArg)(JNIEnv*, jvalue* value, void* data);

/**
 * ArgDisposers are used to dispose arguments after invocation of Java methods.
 * We need to dispose those arguments which have been created as local references,
 * e.g. jenv->NewString(), jenv->NewObjectArray(), jenv->New<Type>Array().
 */
typedef struct JPy_ArgDisposer
{
    void* data;
    JPy_DisposeArg DisposeArg;
}
JPy_ArgDisposer;


struct JPy_ParamDescriptor;

typedef int (*JPy_MatchPyArg)(JNIEnv*, struct JPy_ParamDescriptor*, PyObject*);
typedef int (*JPy_ConvertPyArg)(JNIEnv*, struct JPy_ParamDescriptor*, PyObject*, jvalue*, JPy_ArgDisposer*);

/**
 * Method return value descriptor.
 */
typedef struct JPy_ReturnDescriptor
{
    JPy_JType* type;
}
JPy_ReturnDescriptor;

/**
 * Method parameter descriptor.
 */
typedef struct JPy_ParamDescriptor
{
    JPy_JType* type;
    char isMutable;
    char isReturn;
    JPy_MatchPyArg MatchPyArg;
    JPy_ConvertPyArg ConvertPyArg;
}
JPy_ParamDescriptor;


int JType_Check(PyObject* obj);

JPy_JType* JType_GetTypeForName(JNIEnv* jenv, const char* typeName, jboolean resolve);
JPy_JType* JType_GetType(JNIEnv* jenv, jclass classRef, jboolean resolve);

PyObject* JType_ConvertJavaToPythonObject(JNIEnv* jenv, JPy_JType* type, jobject objectRef);
int       JType_ConvertPythonToJavaObject(JNIEnv* jenv, JPy_JType* type, PyObject* arg, jobject* objectRef);

PyObject* JType_GetOverloadedMethod(JNIEnv* jenv, JPy_JType* type, PyObject* methodName, jboolean useSuperClass);


// Non-API. Defined in jpy_jobj.c
int JType_InitSlots(JPy_JType* type);
// Non-API. Defined in jpy_jtype.c
int JType_ResolveType(JNIEnv* jenv, JPy_JType* type);

#ifdef __cplusplus
}  /* extern "C" */
#endif
#endif /* !JPY_JTYPE_H */