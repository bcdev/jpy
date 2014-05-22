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
#include "jpy_conv.h"



int JPy_AsJObject(JNIEnv* jenv, PyObject* pyObj, jobject* objectRef)
{
    return JType_ConvertPythonToJavaObject(jenv, JPy_JObject, pyObj, objectRef);
}

int JPy_AsJObjectWithType(JNIEnv* jenv, PyObject* pyObj, jobject* objectRef, JPy_JType* type)
{
    return JType_ConvertPythonToJavaObject(jenv, type, pyObj, objectRef);
}

int JPy_AsJObjectWithClass(JNIEnv* jenv, PyObject* pyObj, jobject* objectRef, jclass classRef)
{
    *objectRef = NULL;

    if (pyObj == Py_None) {
        return 0;
    }

    if (classRef != NULL) {
        JPy_JType* valueType;

        valueType = JType_GetType(jenv, classRef, JNI_FALSE);
        if (valueType == NULL) {
            return -1;
        }
        if (JPy_AsJObjectWithType(jenv, pyObj, objectRef, valueType) < 0) {
            return -1;
        }
    } else {
        if (JPy_AsJObject(jenv, pyObj, objectRef) < 0) {
            return -1;
        }
    }

    return 0;
}



PyObject* JPy_FromJObject(JNIEnv* jenv, jobject objectRef)
{
    jclass classRef;
    JPy_JType* type;

    classRef = (*jenv)->GetObjectClass(jenv, objectRef);
    type = JType_GetType(jenv, classRef, JNI_FALSE);
    (*jenv)->DeleteLocalRef(jenv, classRef);
    if (type == NULL) {
        return NULL;
    }

    return JPy_FromJObjectWithType(jenv, objectRef, type);
}

PyObject* JPy_FromJObjectWithType(JNIEnv* jenv, jobject objectRef, JPy_JType* type)
{
    return JType_ConvertJavaToPythonObject(jenv, type, objectRef);
}


/**
 * Copies the UTF, zero-terminated C-string.
 * Caller is responsible for freeing the returned string using PyMem_Del().
 */
char* JPy_CopyUTFString(const char* utfChars)
{
    char* utfCharsCopy;

    utfCharsCopy = PyMem_New(char, strlen(utfChars) + 1);
    if (utfCharsCopy == NULL) {
        PyErr_NoMemory();
        return NULL;
    }

    strcpy(utfCharsCopy, utfChars);
    return utfCharsCopy;
}

/**
 * Copies the given jchar string used by Java into a wchar_t string used by Python.
 * Caller is responsible for freeing the returned string using PyMem_Del().
 */
wchar_t* JPy_ConvertToWCharString(const jchar* jChars, jint length)
{
    wchar_t* wChars;
    jint i;

    wChars = PyMem_New(wchar_t, length + 1);
    if (wChars == NULL) {
        PyErr_NoMemory();
        return NULL;
    }

    for (i = 0; i < length; i++) {
        wChars[i] = (wchar_t) jChars[i];
    }
    wChars[length] = 0;

    return wChars;
}

/**
 * Copies the given wchar_t string used by Python into a jchar string used by Python.
 * Caller is responsible for freeing the returned string using PyMem_Del().
 */
jchar* JPy_ConvertToJCharString(const wchar_t* wChars, jint length)
{
    jchar* jChars;
    jint i;

    jChars = PyMem_New(jchar, length + 1);
    if (jChars == NULL) {
        PyErr_NoMemory();
        return NULL;
    }

    for (i = 0; i < length; i++) {
        jChars[i] = (jchar) wChars[i];
    }
    jChars[length] = (jchar) 0;

    return jChars;
}

/**
 * Gets the UTF name of thie given class.
 * Caller is responsible for freeing the returned string using Py_Del().
 */
char* JPy_GetTypeName(JNIEnv* jenv, jclass classRef)
{
    jstring jTypeName;
    const char* jTypeNameChars;
    char* typeNameCopy;

    jTypeName = (*jenv)->CallObjectMethod(jenv, classRef, JPy_Class_GetName_MID);
    JPy_ON_JAVA_EXCEPTION_RETURN(NULL);

    jTypeNameChars = (*jenv)->GetStringUTFChars(jenv, jTypeName, NULL);
    if (jTypeNameChars == NULL) {
        PyErr_NoMemory();
        typeNameCopy = NULL;
    } else {
        typeNameCopy = JPy_CopyUTFString(jTypeNameChars);
        (*jenv)->ReleaseStringUTFChars(jenv, jTypeName, jTypeNameChars);
    }
    (*jenv)->DeleteLocalRef(jenv, jTypeName);
    return typeNameCopy;
}

/**
 * Gets a string object representing the name of the given class.
 * Returns a new reference.
 */
PyObject* JPy_FromTypeName(JNIEnv* jenv, jclass classRef)
{
    PyObject* pyTypeName;
    jstring jTypeName;
    const char* jTypeNameChars;

    jTypeName = (*jenv)->CallObjectMethod(jenv, classRef, JPy_Class_GetName_MID);
    JPy_ON_JAVA_EXCEPTION_RETURN(NULL);

    jTypeNameChars = (*jenv)->GetStringUTFChars(jenv, jTypeName, NULL);
    JPy_DIAG_PRINT(JPy_DIAG_F_TYPE, "JPy_FromTypeName: classRef=%p, jTypeNameChars=\"%s\"\n", classRef, jTypeNameChars);

    if (jTypeNameChars == NULL) {
        PyErr_NoMemory();
        pyTypeName = NULL;
    } else {
        pyTypeName = Py_BuildValue("s", jTypeNameChars);
        (*jenv)->ReleaseStringUTFChars(jenv, jTypeName, jTypeNameChars);
    }
    (*jenv)->DeleteLocalRef(jenv, jTypeName);
    return pyTypeName;
}

#ifndef IS_PYTHON_3_API
#define PyUnicode_2BYTE_KIND 0
#endif

PyObject* JPy_FromJString(JNIEnv* jenv, jstring stringRef)
{
    PyObject* returnValue;
    const jchar* jChars;
    jint length;

    if (stringRef == NULL) {
        return Py_BuildValue("");
    }

    length = (*jenv)->GetStringLength(jenv, stringRef);
    if (length == 0) {
        return Py_BuildValue("s", "");
    }

    jChars = (*jenv)->GetStringChars(jenv, stringRef, NULL);
    if (jChars == NULL) {
        PyErr_NoMemory();
        return NULL;
    }

    // todo: py27: replace PyUnicode_FromKindAndData() for Python 2.7
    returnValue = PyUnicode_FromKindAndData(PyUnicode_2BYTE_KIND, jChars, length);
    (*jenv)->ReleaseStringChars(jenv, stringRef, jChars);
    return returnValue;
}

/**
 * Returns a new Java string (a local reference).
 */
int JPy_AsJString(JNIEnv* jenv, PyObject* arg, jstring* stringRef)
{
    Py_ssize_t length;
    wchar_t* wChars;

    if (arg == Py_None) {
        *stringRef = NULL;
        return 0;
    }

    // todo: py27: replace PyUnicode_AsWideCharString() for Python 2.7
    wChars = PyUnicode_AsWideCharString(arg, &length);
    if (wChars == NULL) {
        *stringRef = NULL;
        return -1;
    }

    if (sizeof(wchar_t) == sizeof(jchar)) {
        *stringRef = (*jenv)->NewString(jenv, (const jchar*) wChars, length);
    } else {
        jchar* jChars;
        jChars = JPy_ConvertToJCharString(wChars, length);
        if (jChars == NULL) {
            goto error;
        }
        *stringRef = (*jenv)->NewString(jenv, jChars, length);
        PyMem_Del(jChars);
    }
    if (*stringRef == NULL) {
        PyMem_Del(wChars);
        PyErr_NoMemory();
        return -1;
    }

error:
    PyMem_Del(wChars);

    return 0;
}

