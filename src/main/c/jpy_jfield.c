/*
 * Copyright (C) 2014 Brockmann Consult GmbH
 *
 * This program is free software; you can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation; either version 3 of the License,
 * or (at your option) any later version. This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
 * PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program; if not, see
 * http://www.gnu.org/licenses/
 */

#include "jpy_module.h"
#include "jpy_diag.h"
#include "jpy_jtype.h"
#include "jpy_jobj.h"
#include "jpy_jfield.h"
#include "jpy_conv.h"


JPy_JField* JField_New(JPy_JType* declaringClass, PyObject* fieldName, JPy_JType* fieldType, jboolean isStatic, jboolean isFinal, jfieldID fid)
{
    PyTypeObject* type = &JField_Type;
    JPy_JField* field;

    field = (JPy_JField*) type->tp_alloc(type, 0);
    field->declaringClass = declaringClass;
    field->name = fieldName;
    field->type = fieldType;
    field->isStatic = isStatic;
    field->isFinal = isFinal;
    field->fid = fid;

    Py_INCREF(field->name);
    Py_INCREF(field->type);

    return field;
}

/**
 * The JField type's tp_dealloc slot.
 */
void JField_dealloc(JPy_JField* self)
{
    Py_DECREF(self->name);
    Py_DECREF(self->type);
    Py_TYPE(self)->tp_free((PyObject*) self);
}

void JField_Del(JPy_JField* field)
{
    JField_dealloc(field);
}


/**
 * The JField type's tp_repr slot.
 */
PyObject* JField_repr(JPy_JField* self)
{
    // todo: py27: replace PyUnicode_AsUTF8()
    const char* name = PyUnicode_AsUTF8(self->name);
    return PyUnicode_FromFormat("%s(name='%s', is_static=%d, is_final=%d, fid=%p)",
                                ((PyObject*)self)->ob_type->tp_name,
                                name,
                                self->isStatic,
                                self->isFinal,
                                self->fid);
}

/**
 * The JField type's tp_str slot.
 */
PyObject* JField_str(JPy_JField* self)
{
    Py_INCREF(self->name);
    return self->name;
}


static PyMemberDef JField_members[] =
{
    {"name",        T_OBJECT_EX, offsetof(JPy_JField, name),       READONLY, "Field name"},
    {"is_static",   T_BOOL,      offsetof(JPy_JField, isStatic),   READONLY, "Tests if this is a static field"},
    {"is_final",    T_BOOL,      offsetof(JPy_JField, isFinal),    READONLY, "Tests if this is a final field"},
    {NULL}  /* Sentinel */
};


/**
 * Implements the BeamPy_JObjectType class singleton.
 */
PyTypeObject JField_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "jpy.JField",                 /* tp_name */
    sizeof (JPy_JField),          /* tp_basicsize */
    0,                            /* tp_itemsize */
    (destructor)JField_dealloc,   /* tp_dealloc */
    NULL,                         /* tp_print */
    NULL,                         /* tp_getattr */
    NULL,                         /* tp_setattr */
    NULL,                         /* tp_reserved */
    (reprfunc)JField_repr,        /* tp_repr */
    NULL,                         /* tp_as_number */
    NULL,                         /* tp_as_sequence */
    NULL,                         /* tp_as_mapping */
    NULL,                         /* tp_hash  */
    NULL,                         /* tp_call */
    (reprfunc)JField_str,         /* tp_str */
    NULL,                         /* tp_getattro */
    NULL,                         /* tp_setattro */
    NULL,                         /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,           /* tp_flags */
    "Java Field Wrapper",         /* tp_doc */
    NULL,                         /* tp_traverse */
    NULL,                         /* tp_clear */
    NULL,                         /* tp_richcompare */
    0,                            /* tp_weaklistoffset */
    NULL,                         /* tp_iter */
    NULL,                         /* tp_iternext */
    NULL,                         /* tp_methods */
    JField_members,               /* tp_members */
    NULL,                         /* tp_getset */
    NULL,                         /* tp_base */
    NULL,                         /* tp_dict */
    NULL,                         /* tp_descr_get */
    NULL,                         /* tp_descr_set */
    0,                            /* tp_dictoffset */
    NULL,                         /* tp_init */
    NULL,                         /* tp_alloc */
    NULL,                         /* tp_new */
};