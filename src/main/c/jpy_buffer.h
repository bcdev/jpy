#ifndef JPY_BUFFER_H
#define JPY_BUFFER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <Python.h>

PyObject* JPy_ToPrimitiveArrayBufferReadOnly(PyObject* obj, Py_buffer* view, const char* format, int len);
PyObject* JPy_ToPrimitiveArrayBufferWritable(PyObject* obj, Py_buffer* view, const char* format, int len);
PyObject* JPy_ToPrimitiveArrayBuffer(PyObject* obj, Py_buffer* view, int flags, const char* format, int len);

#ifdef __cplusplus
}  /* extern "C" */
#endif
#endif /* !JPY_BUFFER_H */