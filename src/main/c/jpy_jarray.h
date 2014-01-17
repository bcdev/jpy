#ifndef JPY_JARRAY_H
#define JPY_JARRAY_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * The Java primitive array representation in Python.
 *
 * IMPORTANT: JPy_JArray must only differ from the JPy_JObj structure by the 'bufferExportCount' member
 * since we use the same basic type, name JPy_JType for it. DON'T ever change member positions!
 * @see JPy_JObj
 */
typedef struct JPy_JArray
{
    PyObject_HEAD
    jobject objectRef;
    jint bufferExportCount;
}
JPy_JArray;

extern PyBufferProcs JArray_as_buffer_boolean;
extern PyBufferProcs JArray_as_buffer_char;
extern PyBufferProcs JArray_as_buffer_byte;
extern PyBufferProcs JArray_as_buffer_short;
extern PyBufferProcs JArray_as_buffer_int;
extern PyBufferProcs JArray_as_buffer_long;
extern PyBufferProcs JArray_as_buffer_float;
extern PyBufferProcs JArray_as_buffer_double;

#ifdef __cplusplus
}  /* extern "C" */
#endif
#endif /* !JPY_JARRAY_H */
