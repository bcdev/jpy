#include "jpy_compat.h"


#ifdef JPY_COMPAT_27

const char* JPy_AsUTF8_PriorToPy33(PyObject* pyStr)
{
    if (PyUnicode_Check(pyStr)) {
        pyStr = PyUnicode_AsUTF8String(pyStr);
        if (pyStr == NULL) {
            return NULL;
        }
    }
    return PyString_AsString(pyStr);
}

wchar_t* JPy_AsWideCharString_PriorToPy33(PyObject* pyUnicode, Py_ssize_t* size)
{
    wchar_t* buffer = NULL;
    PyObject* pyNewRef = NULL;

    if (!PyUnicode_Check(pyUnicode)) {
        pyNewRef = PyUnicode_FromObject(pyUnicode);
        if (pyNewRef == NULL) {
            goto error;
        }
        pyUnicode = pyNewRef;
    }

    *size = PyUnicode_GET_SIZE(pyUnicode);
    if (*size < 0) {
        goto error;
    }

    buffer = PyMem_New(wchar_t, *size);
    if (buffer == NULL) {
        goto error;
    }

    *size = PyUnicode_AsWideChar((PyUnicodeObject*) pyUnicode, buffer, *size);
    if (*size < 0) {
        PyMem_Free(buffer);
        buffer = NULL;
    }

error:

    Py_XDECREF(pyNewRef);

    return buffer;
}

#endif
