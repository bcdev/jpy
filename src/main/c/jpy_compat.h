#ifndef JPY_COMPAT_H
#define JPY_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <Python.h>


#if PY_MAJOR_VERSION >= 3

#define JPy_IS_CLONG(pyArg)      PyLong_Check(pyArg)
#define JPy_AS_CLONG(pyArg)      PyLong_AsLong(pyArg)
#define JPy_AS_CLONGLONG(pyArg)  PyLong_AsLongLong(pyArg)
#define JPy_FROM_CLONG(cl)       PyLong_FromLong(cl)

// todo: py27: use the following macros where appropriate
#define JPy_IS_STR(pyArg)        PyUnicode_Check(pyArg)
#define JPy_FROM_CSTR(cstr)      PyUnicode_FromString(cstr)
#define JPy_FROM_FORMAT(cstr)    PyUnicode_FromFormat(cstr)

#else

#define JPy_IS_CLONG(pyArg)      (PyInt_Check(pyArg) || PyLong_Check(pyArg))
#define JPy_AS_CLONG(pyArg)      (PyInt_Check(pyArg) ? PyInt_AsLong(pyArg) : PyLong_AsLong(pyArg))
#define JPy_AS_CLONGLONG(pyArg)  (PyInt_Check(pyArg) ? PyInt_AsLong(pyArg) : PyLong_AsLongLong(pyArg))
#define JPy_FROM_CLONG(cl)        PyInt_FromLong(cl)

// todo: py27: use the following macros where appropriate
#define JPy_IS_STR(pyArg)        (PyString_Check(pyArg) || PyUnicode_Check(pyArg))
#define JPy_FROM_CSTR(cstr)      PyString_FromString(cstr)
#define JPy_FROM_FORMAT(cstr)    PyString_FromFormat(cstr)

#endif



#if PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION >= 3

#define JPy_AS_UTF8(unicode)                 PyUnicode_AsUTF8(unicode)
#define JPy_AS_WIDE_CHAR_STR(unicode, size)  PyUnicode_AsWideCharString(unicode, size)
#define JPy_FROM_WIDE_CHAR_STR(wc, size)     PyUnicode_FromKindAndData(PyUnicode_2BYTE_KIND, wc, size)

#else

// Implement conversion rules from Python 2 to 3 as given here:
// https://docs.python.org/3.3/howto/cporting.html
// http://lucumr.pocoo.org/2011/1/22/forwards-compatible-python/

const char* JPy_AsUTF8_PriorToPy33(PyObject* unicode);
wchar_t* JPy_AsWideCharString_PriorToPy33(PyObject *unicode, Py_ssize_t *size);

#define JPy_AS_UTF8(unicode)                 JPy_AsUTF8_PriorToPy33(unicode)
#define JPy_AS_WIDE_CHAR_STR(unicode, size)  JPy_AsWideCharString_PriorToPy33(unicode, size)
#define JPy_FROM_WIDE_CHAR_STR(wc, size)     PyUnicode_FromWideChar(wc, size)

#endif


#ifdef __cplusplus
} /* extern "C" */
#endif
#endif /* !JPY_COMPAT_H */