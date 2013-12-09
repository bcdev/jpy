#include "jpy_buffer.h"
#include "jpy_carray.h"

PyObject* JPy_ToPrimitiveArrayBuffer(PyObject* obj, Py_buffer* view, int flags, const char* format, int len)
{
    if (obj == NULL || obj == Py_None) {
        if (len <= 0) {
            PyErr_SetString(PyExc_ValueError, "no buffer length specified");
            return NULL;
        }
        obj = CArray_New(format, len);
        if (obj == NULL) {
            return PyErr_NoMemory();
        }
    }

    if (PyObject_CheckBuffer(obj)) {
        if (PyObject_GetBuffer(obj, view, flags) == 0) {
            if (view->ndim <= 1 && (len < 0 || view->len / view->itemsize >= len)) {
                Py_INCREF(obj);
                return obj;
            } else {
                //printf("ndim=%d, len=%d, itemsize=%d, expected len=%d\n", view->ndim, view->len, view->itemsize, len);
                PyBuffer_Release(view);
                PyErr_SetString(PyExc_ValueError, "illegal buffer configuration");
                return NULL;
            }
        }  else {
            PyErr_SetString(PyExc_TypeError, "failed to access buffer");
            return NULL;
        }
    } else {
        PyErr_SetString(PyExc_TypeError, "buffer type expected");
        return NULL;
    }
}

PyObject* JPy_ToPrimitiveArrayBufferReadOnly(PyObject* obj, Py_buffer* view, const char* format, int len)
{
    return JPy_ToPrimitiveArrayBuffer(obj, view, PyBUF_SIMPLE, format, len);
}


PyObject* JPy_ToPrimitiveArrayBufferWritable(PyObject* obj, Py_buffer* view, const char* format, int len)
{
    return JPy_ToPrimitiveArrayBuffer(obj, view, PyBUF_WRITABLE, format, len);
}
