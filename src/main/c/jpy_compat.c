#include "jpy_compat.h"


const char* JPy_AsUTF8_PriorToPy33(PyObject* pyStr)
{
    if (PyUnicode_Check(pyStr)) {
        pyStr = PyUnicode_AsUTF8String(pyStr);
    }
    if (pyStr != NULL) {
        return PyString_AsString(pyStr);
    } else {
        return NULL;
    }
}

// todo - py27: interim solution: JPy_AsWideCharString_PriorToPy33() is not correctly implemented, this is just for compatibility testing
wchar_t* JPy_AsWideCharString_PriorToPy33(PyObject* pyUnicode, Py_ssize_t* size)
{
    static wchar_t buffer[4096];

    if (!PyUnicode_Check(pyUnicode)) {
        pyUnicode = PyUnicode_FromObject(pyUnicode);
    }
    *size = PyUnicode_AsWideChar((PyObject*) pyUnicode, buffer, 4096);
    return buffer;
}
