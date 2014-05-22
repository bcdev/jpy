#ifndef JPY_COMPAT_H
#define JPY_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <Python.h>

// Implement conversion rules from Python 2 to 3 as given here:
// https://docs.python.org/3.3/howto/cporting.html
// http://lucumr.pocoo.org/2011/1/22/forwards-compatible-python/

const char* JPy_AsUTF8_PriorToPy33(PyObject* unicode);
wchar_t* JPy_AsWideCharString_PriorToPy33(PyObject *unicode, Py_ssize_t *size);


#if PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION >= 3
#define JPy_AS_UTF8(unicode)                 PyUnicode_AsUTF8(unicode)
#define JPy_AS_WIDE_CHAR_STR(unicode, size)  PyUnicode_AsWideCharString(unicode, size)
#define JPy_FROM_WIDE_CHAR_STR(wc, size)     PyUnicode_FromKindAndData(PyUnicode_2BYTE_KIND, wc, size)
#else
#define JPy_AS_UTF8(unicode)                 JPy_AsUTF8_PriorToPy33(unicode)
#define JPy_AS_WIDE_CHAR_STR(unicode, size)  JPy_AsWideCharString_PriorToPy33(unicode, size)
#define JPy_FROM_WIDE_CHAR_STR(wc, size)     PyUnicode_FromWideChar(wc, size)
#endif

// todo: py27: Check if the following snippet is applicable. It is from StackOverflow
//
// http://stackoverflow.com/questions/4352847/py-major-version-undefined-when-compiling-python-c-module-for-python-3-x
//
//<<<<<<

#if PY_MAJOR_VERSION >= 3
#define PYSTR_FROMC PyUnicode_FromString
#define PYSTR_FORMAT PyUnicode_Format
#define PYINT_FROMC PyLong_FromLong
#else
#define PYSTR_FROMC PyString_FromString
#define PYSTR_FORMAT PyString_Format
#define PYINT_FROMC PyInt_FromLong
#endif

//>>>>>>

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif /* !JPY_COMPAT_H */