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

#ifndef JPY_JOBJ_H
#define JPY_JOBJ_H

#ifdef __cplusplus
extern "C" {
#endif

#include "jpy_compat.h"

/**
 * The Java Object representation in Python.
 * @see JPy_JArray
 */
typedef struct JPy_JObj
{
    PyObject_HEAD
    jobject objectRef;
}
JPy_JObj;


int JObj_Check(PyObject* arg);

JPy_JObj* JObj_New(JNIEnv* jenv, jobject objectRef);
JPy_JObj* JObj_FromType(JNIEnv* jenv, JPy_JType* type, jobject objectRef);

int JObj_InitTypeSlots(PyTypeObject* type, const char* typeName, PyTypeObject* superType);


#ifdef __cplusplus
}  /* extern "C" */
#endif
#endif /* !JPY_JOBJ_H */