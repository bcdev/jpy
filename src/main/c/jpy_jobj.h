#ifndef JPY_JOBJ_H
#define JPY_JOBJ_H

#ifdef __cplusplus
extern "C" {
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// JClass

/**
 * The Java Object representation in Python.
 * @see JPy_JArray
 */
typedef struct JPy_JObj
{
    PyObject_HEAD
    jobject objectRef;
}
JPy_JObj;


int JObj_Check(PyObject* arg);

JPy_JObj* JObj_New(JNIEnv* jenv, jobject objectRef);
JPy_JObj* JObj_FromType(JNIEnv* jenv, JPy_JType* type, jobject objectRef);

int JObj_InitTypeSlots(PyTypeObject* type, const char* typeName, PyTypeObject* superType);


#ifdef __cplusplus
}  /* extern "C" */
#endif
#endif /* !JPY_JOBJ_H */