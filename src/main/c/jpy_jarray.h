#ifndef JPY_JARRAY_H
#define JPY_JARRAY_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * The Java primitive array representation in Python.
 *
 * IMPORTANT: It must only differ from the JPy_JObj type by the 'exportCount' member
 * since we use the same basic type, name JPy_JType for it.
 */
typedef struct JPy_JArray
{
    PyObject_HEAD
    jarray arrayRef;
    jint exportCount;
}
JPy_JArray;

PyObject* JArray_New(const char* format, int itemCount);

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
