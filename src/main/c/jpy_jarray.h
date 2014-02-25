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

#ifndef JPY_JARRAY_H
#define JPY_JARRAY_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * The Java primitive array representation in Python.
 *
 * IMPORTANT: JPy_JArray must only differ from the JPy_JObj structure by the 'bufferExportCount' member
 * since we use the same basic type, name JPy_JType for it. DON'T ever change member positions!
 * @see JPy_JObj
 */
typedef struct JPy_JArray
{
    PyObject_HEAD
    jobject objectRef;
    jint bufferExportCount;
}
JPy_JArray;

extern PyBufferProcs JArray_as_buffer_boolean;
extern PyBufferProcs JArray_as_buffer_char;
extern PyBufferProcs JArray_as_buffer_byte;
extern PyBufferProcs JArray_as_buffer_short;
extern PyBufferProcs JArray_as_buffer_int;
extern PyBufferProcs JArray_as_buffer_long;
extern PyBufferProcs JArray_as_buffer_float;
extern PyBufferProcs JArray_as_buffer_double;

#ifdef __cplusplus
}  /* extern "C" */
#endif
#endif /* !JPY_JARRAY_H */
