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

#ifndef JPY_JFIELD_H
#define JPY_JFIELD_H

#ifdef __cplusplus
extern "C" {
#endif

#include "jpy_compat.h"

/**
 * Python object representing a Java method. It's type is 'JMethod'.
 */
typedef struct
{
    PyObject_HEAD

    // The declaring class.
    JPy_JType* declaringClass;
    // Field name.
    PyObject* name;
    // Field type.
    JPy_JType* type;
    // Method is static?
    char isStatic;
    // Method is final?
    char isFinal;
    // Field ID retrieved from JNI.
    jfieldID fid;
}
JPy_JField;

/**
 * The Python 'JMethod' type singleton.
 */
extern PyTypeObject JField_Type;

JPy_JField* JField_New(JPy_JType* declaringType, PyObject* fieldKey, JPy_JType* fieldType, jboolean isStatic, jboolean isFinal, jfieldID fid);
void JField_Del(JPy_JField* field);

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* !JPY_JFIELD_H */