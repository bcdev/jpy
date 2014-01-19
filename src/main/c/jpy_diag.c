#include <Python.h>
#include "structmember.h"
#include "jpy_diag.h"

int JPy_ActiveDiagFlags = JPy_DIAG_OFF;

void JPy_DiagPrint(int diagFlags, const char * format, ...)
{
    if ((JPy_ActiveDiagFlags & diagFlags) != 0) {
        va_list args;
        va_start(args, format);
        vfprintf(stdout, format, args);
        va_end(args);
    }
}

PyObject* DiagFlags_New()
{
    JPy_DiagFlags* self;

    self = (JPy_DiagFlags*) PyObject_New(PyObject, &DiagFlags_Type);

    self->off   = JPy_DIAG_OFF;
    self->type  = JPy_DIAG_TYPE;
    self->meth  = JPy_DIAG_METH;
    self->exec  = JPy_DIAG_EXEC;
    self->mem   = JPy_DIAG_MEM;
    self->all   = JPy_DIAG_ALL;

    return (PyObject*) self;
}


PyObject* DiagFlags_getattro(JPy_DiagFlags* self, PyObject *attr_name)
{
    if (strcmp(PyUnicode_AsUTF8(attr_name), "value") == 0) {
        return PyLong_FromLong(JPy_ActiveDiagFlags);
    } else {
        return PyObject_GenericGetAttr((PyObject*) self, attr_name);
    }
}


int DiagFlags_setattro(JPy_DiagFlags* self, PyObject *attr_name, PyObject *v)
{
    if (strcmp(PyUnicode_AsUTF8(attr_name), "value") == 0) {
        if (v == Py_None || v == Py_False) {
            JPy_ActiveDiagFlags = JPy_DIAG_OFF;
        } else if (v == Py_True) {
            JPy_ActiveDiagFlags = JPy_DIAG_ALL;
        } else if (PyLong_Check(v)) {
            JPy_ActiveDiagFlags = (int) PyLong_AsLong(v);
        } else {
            PyErr_SetString(PyExc_ValueError, "invalid flags value");
            return -1;
        }
        return 0;
    } else {
        return PyObject_GenericSetAttr((PyObject*) self, attr_name, v);
    }
}

static PyMemberDef DiagFlags_members[] =
{
    {"off",    T_INT, offsetof(JPy_DiagFlags, off),   READONLY, "Don't print any diagnostic messages"},
    {"type",   T_INT, offsetof(JPy_DiagFlags, type),  READONLY, "Type resolution: print diagnostic messages while generating Python classes from Java classes"},
    {"meth",   T_INT, offsetof(JPy_DiagFlags, meth),  READONLY, "Method resolution: print diagnostic messages while resolving Java overloaded methods"},
    {"exec",   T_INT, offsetof(JPy_DiagFlags, exec),  READONLY, "Execution: print diagnostic messages when Java code is executed"},
    {"mem",    T_INT, offsetof(JPy_DiagFlags, mem),   READONLY, "Memory: print diagnostic messages when wrapped Java objects are allocated/deallocated"},
    {"all",    T_INT, offsetof(JPy_DiagFlags, all),   READONLY, "Print all diagnostic messages"},
    {NULL}  /* Sentinel */
};


PyTypeObject DiagFlags_Type =
{
    PyVarObject_HEAD_INIT(NULL, 0)
    "jpy.DiagFlags",              /* tp_name */
    sizeof (JPy_DiagFlags),       /* tp_basicsize */
    0,                            /* tp_itemsize */
    NULL,                         /* tp_dealloc */
    NULL,                         /* tp_print */
    NULL,                         /* tp_getattr */
    NULL,                         /* tp_setattr */
    NULL,                         /* tp_reserved */
    NULL,                         /* tp_repr */
    NULL,                         /* tp_as_number */
    NULL,                         /* tp_as_sequence */
    NULL,                         /* tp_as_mapping */
    NULL,                         /* tp_hash  */
    NULL,                         /* tp_call */
    NULL,                         /* tp_str */
    (getattrofunc) DiagFlags_getattro,  /* tp_getattro */
    (setattrofunc) DiagFlags_setattro,  /* tp_setattro */
    NULL,                         /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,            /* tp_flags */
    "Diagnostic flags for debugging",   /* tp_doc */
    NULL,                         /* tp_traverse */
    NULL,                         /* tp_clear */
    NULL,                         /* tp_richcompare */
    0,                            /* tp_weaklistoffset */
    NULL,                         /* tp_iter */
    NULL,                         /* tp_iternext */
    NULL,                         /* tp_methods */
    DiagFlags_members,            /* tp_members */
    NULL,                         /* tp_getset */
    NULL,                         /* tp_base */
    NULL,                         /* tp_dict */
    NULL,                         /* tp_descr_get */
    NULL,                         /* tp_descr_set */
    0,                            /* tp_dictoffset */
    (initproc) NULL,              /* tp_init */
    NULL,                         /* tp_alloc */
    NULL,                         /* tp_new */
};


