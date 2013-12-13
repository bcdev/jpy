#ifndef JPY_JFIELD_H
#define JPY_JFIELD_H

#ifdef __cplusplus
extern "C" {
#endif


/**
 * Python object representing a Java method. It's type is 'JMethod'.
 */
typedef struct
{
    PyObject_HEAD

    // The declaring class.
    JPy_JType* declaringClass;
    // Field name.
    PyObject* name;
    // Field type.
    JPy_JType* type;
    // Method is static?
    char isStatic;
    // Method is final?
    char isFinal;
    // Field ID retrieved from JNI.
    jfieldID fid;
}
JPy_JField;

/**
 * The Python 'JMethod' type singleton.
 */
extern PyTypeObject JField_Type;

JPy_JField* JField_New(JPy_JType* declaringType, PyObject* fieldKey, JPy_JType* fieldType, jboolean isStatic, jboolean isFinal, jfieldID fid);
void JField_Del(JPy_JField* field);

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* !JPY_JFIELD_H */