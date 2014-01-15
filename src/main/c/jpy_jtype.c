#include "jpy_module.h"
#include "jpy_jtype.h"
#include "jpy_jmethod.h"
#include "jpy_jfield.h"
#include "jpy_jobj.h"
#include "jpy_carray.h"
#include "jpy_conv.h"


JPy_JType* JType_New(JNIEnv* jenv, jclass classRef, jboolean resolve);
int JType_ResolveType(JNIEnv* jenv, JPy_JType* type);
int JType_InitComponentType(JNIEnv* jenv, JPy_JType* type, jboolean resolve);
int JType_InitSuperType(JNIEnv* jenv, JPy_JType* type, jboolean resolve);
int JType_ProcessClassConstructors(JNIEnv* jenv, JPy_JType* type);
int JType_ProcessClassFields(JNIEnv* jenv, JPy_JType* type);
int JType_ProcessClassMethods(JNIEnv* jenv, JPy_JType* type);
int JType_AddMethod(JPy_JType* type, JPy_JMethod* method);
JPy_ReturnDescriptor* JType_CreateReturnDescriptor(JNIEnv* jenv, jclass returnType);
JPy_ParamDescriptor* JType_CreateParamDescriptors(JNIEnv* jenv, int paramCount, jarray paramTypes);
void JType_InitParamDescriptorFunctions(JPy_ParamDescriptor* paramDescriptor);
void JType_InitMethodParamDescriptorFunctions(JPy_JType* type, JPy_JMethod* method);
int JType_ProcessField(JNIEnv* jenv, JPy_JType* declaringType, PyObject* fieldKey, const char* fieldName, jclass fieldClassRef, jboolean isStatic, jboolean isFinal, jfieldID fid);
int JType_DisposeLocalObjectRefArg(JNIEnv* jenv, jvalue* value, void* data);
int JType_DisposeReadOnlyBufferArg(JNIEnv* jenv, jvalue* value, void* data);
int JType_DisposeWritableBufferArg(JNIEnv* jenv, jvalue* value, void* data);

JPy_JType* JType_GetTypeForName(JNIEnv* jenv, const char* typeName, jboolean resolve)
{
    const char* resourceName;
    jclass classRef;

    if (strchr(typeName, '.') != NULL) {
        // resourceName: Replace dots '.' by slashes '/'
        char* c;
        resourceName = PyMem_New(char, strlen(typeName) + 1);
        if (resourceName == NULL) {
            PyErr_NoMemory();
            return NULL;
        }
        strcpy((char*) resourceName, typeName);
        c = (char*) resourceName;
        while ((c = strchr(c, '.')) != NULL) {
            *c = '/';
        }
    } else {
        resourceName = typeName;
    }

    JPy_DEBUG_PRINTF("JType_GetTypeForName: typeName='%s', resourceName='%s'\n", typeName, resourceName);

    classRef = (*jenv)->FindClass(jenv, resourceName);

    if (typeName != resourceName) {
        PyMem_Del((char*) resourceName);
    }

    if (classRef == NULL) {
        PyErr_SetString(PyExc_ValueError, "Java class not found");
        return NULL;
    }

    return JType_GetType(jenv, classRef, resolve);
}

/**
 * Returns a new reference.
 */
JPy_JType* JType_GetType(JNIEnv* jenv, jclass classRef, jboolean resolve)
{
    PyObject* typeKey;
    PyObject* typeValue;
    JPy_JType* type;

    if (JPy_Types == NULL) {
        PyErr_SetString(PyExc_RuntimeError, "internal error: module 'jpy' not initialized");
        return NULL;
    }

    typeKey = JPy_FromTypeName(jenv, classRef);
    typeValue = PyDict_GetItem(JPy_Types, typeKey);
    if (typeValue == NULL) {

        // Create a new type instance
        type = JType_New(jenv, classRef, resolve);
        if (type == NULL) {
            Py_DECREF(typeKey);
            return NULL;
        }

        // In order to avoid infinite recursion, we have to register the new type first...
        PyDict_SetItem(JPy_Types, typeKey, (PyObject*) type);

        // ... before we can continue processing the super type ...
        if (JType_InitSuperType(jenv, type, resolve) < 0) {
            PyDict_DelItem(JPy_Types, typeKey);
            return NULL;
        }

        // ... and processing the component type.
        if (JType_InitComponentType(jenv, type, resolve) < 0) {
            PyDict_DelItem(JPy_Types, typeKey);
            return NULL;
        }

        // Finally we initialise the type's slots, so that our JObj instances behave pythonic.
        if (JType_InitSlots(type) < 0) {
            JPy_DEBUG_PRINTF("JType_GetType: error: JType_InitSlots() failed for javaName='%s'\n", type->javaName);

            PyDict_DelItem(JPy_Types, typeKey);

            //printf("JType_GetType: after PyDict_DelItem\n");

            return NULL;
        }

    } else {
        Py_DECREF(typeKey);

        if (!JType_Check(typeValue)) {
            PyErr_Format(PyExc_RuntimeError, "attributes in 'jpy." JPy_MODULE_ATTR_NAME_TYPES "' must be of type 'jpy.JType'");
            return NULL;
        }

        type = (JPy_JType*) typeValue;
    }

    JPy_DEBUG_PRINTF("JType_GetType: javaName='%s', resolve=%d, resolved=%d, type=%p\n", type->javaName, resolve, type->isResolved, type);

    if (!type->isResolved && resolve) {
        if (JType_ResolveType(jenv, type) < 0) {
            return NULL;
        }
    }

    return type;
}

/**
 * Creates a type instance of the meta type 'JType_Type'.
 * Such type instances are used as types for Java Objects in Python.
 */
JPy_JType* JType_New(JNIEnv* jenv, jclass classRef, jboolean resolve)
{
    PyTypeObject* metaType;
    JPy_JType* type;

    metaType = &JType_Type;

    type = (JPy_JType*) metaType->tp_alloc(metaType, 0);
    if (type == NULL) {
        return NULL;
    }

    type->classRef = NULL;
    type->isResolved = JNI_FALSE;
    type->isResolving = JNI_FALSE;

    type->javaName = JPy_GetTypeName(jenv, classRef);
    if (type->javaName == NULL) {
        metaType->tp_free(type);
        return NULL;
    }
    type->typeObj.tp_name = type->javaName;

    type->classRef = (*jenv)->NewGlobalRef(jenv, classRef);
    if (type->classRef == NULL) {
        PyMem_Del(type->javaName);
        type->javaName = NULL;
        metaType->tp_free(type);
        PyErr_NoMemory();
        return NULL;
    }

    type->isPrimitive = (*jenv)->CallBooleanMethod(jenv, type->classRef, JPy_Class_IsPrimitive_MID);

    JPy_DEBUG_PRINTF("JType_New: javaName='%s', resolve=%d, type=%p\n", type->javaName, resolve, type);

    return type;
}

PyObject* JType_ConvertJavaToPythonObject(JNIEnv* jenv, JPy_JType* type, jobject objectRef)
{
    if (objectRef == NULL) {
        return JPy_FROM_JNULL();
    }

    if (type->componentType == NULL) {
        // Scalar type, not an array
        if (type == JPy_JBooleanObj) {
            jboolean value = (*jenv)->CallBooleanMethod(jenv, objectRef, JPy_Boolean_BooleanValue_MID);
            JPy_ON_JAVA_EXCEPTION_RETURN(NULL);
            return JPy_FROM_JBOOLEAN(value);
        } else if (type == JPy_JCharacterObj) {
            jchar value = (*jenv)->CallCharMethod(jenv, objectRef, JPy_Character_CharValue_MID);
            JPy_ON_JAVA_EXCEPTION_RETURN(NULL);
            return JPy_FROM_JCHAR(value);
        } else if (type == JPy_JByteObj || type == JPy_JShortObj || type == JPy_JIntegerObj) {
            jint value = (*jenv)->CallIntMethod(jenv, objectRef, JPy_Number_IntValue_MID);
            JPy_ON_JAVA_EXCEPTION_RETURN(NULL);
            return JPy_FROM_JINT(value);
        } else if (type == JPy_JLongObj) {
            jlong value = (*jenv)->CallLongMethod(jenv, objectRef, JPy_Number_LongValue_MID);
            JPy_ON_JAVA_EXCEPTION_RETURN(NULL);
            return JPy_FROM_JLONG(value);
        } else if (type == JPy_JFloatObj || type == JPy_JDoubleObj) {
            jdouble value = (*jenv)->CallDoubleMethod(jenv, objectRef, JPy_Number_DoubleValue_MID);
            JPy_ON_JAVA_EXCEPTION_RETURN(NULL);
            return JPy_FROM_JDOUBLE(value);
        } else if (type == JPy_JString) {
            return JPy_FromJString(jenv, objectRef);
        } else {
            return (PyObject*) JObj_FromType(jenv, type, objectRef);
        }
    } else if (type->componentType->isPrimitive) {
        // Primitive array
        JPy_CArray* array;
        const char* format;
        jint length;
        jbyte* items;

        length = (*jenv)->GetArrayLength(jenv, objectRef);
        //printf("JType_ConvertJavaToPythonObject: length=%d\n", length);
        items = (*jenv)->GetPrimitiveArrayCritical(jenv, objectRef, 0);
        if (items == NULL) {
            PyErr_NoMemory();
            return NULL;
        }
        if (type->componentType == JPy_JBoolean) {
            format = "b";
        } else if (type->componentType == JPy_JChar) {
            format = "H";
        } else if (type->componentType == JPy_JByte) {
            format = "b";
        } else if (type->componentType == JPy_JShort) {
            format = "h";
        } else if (type->componentType == JPy_JInt) {
            format = "l";
        } else if (type->componentType == JPy_JLong) {
            format = "q";
        } else if (type->componentType == JPy_JFloat) {
            format = "f";
        } else if (type->componentType == JPy_JDouble) {
            format = "d";
        } else {
            (*jenv)->ReleasePrimitiveArrayCritical(jenv, objectRef, items, 0);
            PyErr_SetString(PyExc_RuntimeError, "internal error: unknown primitive Java type");
            return NULL;
        }

        array = (JPy_CArray*) CArray_New(format, length);
        if (array != NULL) {
            memcpy(array->items, items, array->itemSize * length);
        }

        (*jenv)->ReleasePrimitiveArrayCritical(jenv, objectRef, items, 0);

        return (PyObject*) array;
    } else {
        // Object array
        // Note: we may convert the Java array into a list here
        return (PyObject*) JObj_FromType(jenv, type, objectRef);
    }
}

int JType_PythonToJavaConversionError(JPy_JType* type, PyObject* pyArg)
{
    PyErr_Format(PyExc_ValueError, "cannot convert a Python %s to a Java %s", Py_TYPE(pyArg)->tp_name, type->javaName);
    return -1;
}

int JType_CreateJavaObject(JNIEnv* jenv, JPy_JType* type, PyObject* pyArg, jclass classRef, jmethodID initMID, jvalue value, jobject* objectRef)
{
    *objectRef = (*jenv)->NewObjectA(jenv, classRef, initMID, &value);
    if (*objectRef == NULL) {
        PyErr_NoMemory();
        return -1;
    }
    JPy_ON_JAVA_EXCEPTION_RETURN(-1);
    return 0;
}

int JType_CreateJavaBooleanObject(JNIEnv* jenv, JPy_JType* type, PyObject* pyArg, jobject* objectRef)
{
    jvalue value;
    if (PyBool_Check(pyArg) || PyLong_Check(pyArg)) {
        value.z = JPy_AS_JBOOLEAN(pyArg);
    } else {
        return JType_PythonToJavaConversionError(type, pyArg);
    }
    return JType_CreateJavaObject(jenv, type, pyArg, JPy_Boolean_JClass, JPy_Boolean_Init_MID, value, objectRef);
}

int JType_CreateJavaCharacterObject(JNIEnv* jenv, JPy_JType* type, PyObject* pyArg, jobject* objectRef)
{
    jvalue value;
    if (PyLong_Check(pyArg)) {
        value.c = JPy_AS_JCHAR(pyArg);
    } else {
        return JType_PythonToJavaConversionError(type, pyArg);
    }
    return JType_CreateJavaObject(jenv, type, pyArg, JPy_Character_JClass, JPy_Character_Init_MID, value, objectRef);
}

int JType_CreateJavaByteObject(JNIEnv* jenv, JPy_JType* type, PyObject* pyArg, jobject* objectRef)
{
    jvalue value;
    if (PyLong_Check(pyArg)) {
        value.b = JPy_AS_JBYTE(pyArg);
    } else {
        return JType_PythonToJavaConversionError(type, pyArg);
    }
    return JType_CreateJavaObject(jenv, type, pyArg, JPy_Byte_JClass, JPy_Byte_Init_MID, value, objectRef);
}

int JType_CreateJavaShortObject(JNIEnv* jenv, JPy_JType* type, PyObject* pyArg, jobject* objectRef)
{
    jvalue value;
    if (PyLong_Check(pyArg)) {
        value.s = JPy_AS_JSHORT(pyArg);
    } else {
        return JType_PythonToJavaConversionError(type, pyArg);
    }
    return JType_CreateJavaObject(jenv, type, pyArg, JPy_Short_JClass, JPy_Short_Init_MID, value, objectRef);
}

int JType_CreateJavaIntegerObject(JNIEnv* jenv, JPy_JType* type, PyObject* pyArg, jobject* objectRef)
{
    jvalue value;
    if (PyLong_Check(pyArg)) {
        value.i = JPy_AS_JINT(pyArg);
    } else {
        return JType_PythonToJavaConversionError(type, pyArg);
    }
    return JType_CreateJavaObject(jenv, type, pyArg, JPy_Integer_JClass, JPy_Integer_Init_MID, value, objectRef);
}

int JType_CreateJavaLongObject(JNIEnv* jenv, JPy_JType* type, PyObject* pyArg, jobject* objectRef)
{
    jvalue value;
    if (PyLong_Check(pyArg)) {
        value.j = JPy_AS_JLONG(pyArg);
    } else {
        return JType_PythonToJavaConversionError(type, pyArg);
    }
    return JType_CreateJavaObject(jenv, type, pyArg, JPy_Long_JClass, JPy_Long_Init_MID, value, objectRef);
}

int JType_CreateJavaFloatObject(JNIEnv* jenv, JPy_JType* type, PyObject* pyArg, jobject* objectRef)
{
    jvalue value;
    if (PyLong_Check(pyArg)) {
        value.f = (jfloat) JPy_AS_JLONG(pyArg);
    } else if (PyFloat_Check(pyArg)) {
        value.f = JPy_AS_JFLOAT(pyArg);
    } else {
        return JType_PythonToJavaConversionError(type, pyArg);
    }
    return JType_CreateJavaObject(jenv, type, pyArg, JPy_Float_JClass, JPy_Float_Init_MID, value, objectRef);
}

int JType_CreateJavaDoubleObject(JNIEnv* jenv, JPy_JType* type, PyObject* pyArg, jobject* objectRef)
{
    jvalue value;
    if (PyLong_Check(pyArg)) {
        value.d = (jdouble) JPy_AS_JLONG(pyArg);
    } else if (PyFloat_Check(pyArg)) {
        value.d = JPy_AS_JDOUBLE(pyArg);
    } else {
        return JType_PythonToJavaConversionError(type, pyArg);
    }
    return JType_CreateJavaObject(jenv, type, pyArg, JPy_Double_JClass, JPy_Double_Init_MID, value, objectRef);
}

int JType_CreateJavaObjectArray(JNIEnv* jenv, JPy_JType* type, PyObject* pyArg, jobject* objectRef)
{
    PyObject* pyItem;
    jobject jItem;
    jobjectArray array;
    jint index;
    jint itemCount;

    if (!PySequence_Check(pyArg)) {
        return JType_PythonToJavaConversionError(type, pyArg);
    }

    itemCount = PySequence_Length(pyArg);
    if (itemCount < 0) {
        return -1;
    }

    array = (*jenv)->NewObjectArray(jenv, itemCount, type->componentType->classRef, NULL);
    if (array == NULL) {
        PyErr_NoMemory();
        return -1;
    }

    for (index = 0; index < itemCount; index++) {
         pyItem = PySequence_GetItem(pyArg, index);
         if (pyItem == NULL) {
             Py_DECREF(pyItem);
             (*jenv)->DeleteLocalRef(jenv, array);
             return -1;
         }
         if (JType_ConvertPythonToJavaObject(jenv, type->componentType, pyItem, &jItem) < 0) {
             Py_DECREF(pyItem);
             (*jenv)->DeleteLocalRef(jenv, array);
             return -1;
         }
         (*jenv)->SetObjectArrayElement(jenv, array, index, jItem);
         if ((*jenv)->ExceptionCheck(jenv)) {
             Py_DECREF(pyItem);
             (*jenv)->DeleteLocalRef(jenv, array);
             JPy_HandleJavaException(jenv);
             return -1;
         }
         Py_DECREF(pyItem);
    }

    *objectRef = array;
    return 0;
}

int JType_CreateJavaPrimitiveArray(JNIEnv* jenv, JPy_JType* type, PyObject* pyArg, jobject* objectRef)
{
    jint itemSize;
    jint itemCount;
    jarray array;

    if (!PySequence_Check(pyArg)) {
        return JType_PythonToJavaConversionError(type, pyArg);
    }

    itemSize = 0;
    itemCount = PySequence_Length(pyArg);
    if (itemCount < 0) {
        return -1;
    }

    if (type->componentType == JPy_JBoolean) {
        array = (*jenv)->NewBooleanArray(jenv, itemCount);
        itemSize = sizeof(jboolean);
    } else if (type->componentType == JPy_JByte) {
        array = (*jenv)->NewByteArray(jenv, itemCount);
        itemSize = sizeof(jbyte);
    } else if (type->componentType == JPy_JChar) {
        array = (*jenv)->NewCharArray(jenv, itemCount);
        itemSize = sizeof(jchar);
    } else if (type->componentType == JPy_JShort) {
        array = (*jenv)->NewShortArray(jenv, itemCount);
        itemSize = sizeof(jshort);
    } else if (type->componentType == JPy_JInt) {
        array = (*jenv)->NewIntArray(jenv, itemCount);
        itemSize = sizeof(jint);
    } else if (type->componentType->componentType == JPy_JLong) {
        array = (*jenv)->NewLongArray(jenv, itemCount);
        itemSize = sizeof(jlong);
    } else if (type->componentType == JPy_JFloat) {
        array = (*jenv)->NewFloatArray(jenv, itemCount);
        itemSize = sizeof(jfloat);
    } else if (type->componentType == JPy_JDouble) {
        array = (*jenv)->NewDoubleArray(jenv, itemCount);
        itemSize = sizeof(jdouble);
    } else {
        PyErr_SetString(PyExc_RuntimeError, "internal error: illegal primitive type");
        return -1;
    }

    JPy_DEBUG_PRINTF("JType_CreateJavaPrimitiveArray: NOT IMPLEMENTED! componentType=%s, itemCount=%d, itemSize=%d\n", type->componentType->javaName, itemCount, itemSize);

    *objectRef = array;
    return 0;
}


int JType_ConvertPythonToJavaObject(JNIEnv* jenv, JPy_JType* type, PyObject* pyArg, jobject* objectRef)
{
    // todo: Problem of memory leak here: if a new local reference is created here and assigned to *objectRef, the reference escapes.
    //       We have to call (*jenv)->DeleteLocalRef(jenv, *objectRef) some time later

    if (pyArg == Py_None) {
        // None converts into NULL
        *objectRef = NULL;
        return 0;
    } else if (JObj_Check(pyArg)) {
        // If it is already a Java object wrapper JObj, then we are done
        *objectRef = ((JPy_JObj*) pyArg)->objectRef;
        return 0;
    } else if (type->componentType != NULL) {
        // For any other Python argument create a Java object (a new local reference)
        if (type->componentType->isPrimitive) {
            return JType_CreateJavaPrimitiveArray(jenv, type, pyArg, objectRef);
        } else {
            return JType_CreateJavaObjectArray(jenv, type, pyArg, objectRef);
        }
    } else if (type == JPy_JBoolean || type == JPy_JBooleanObj) {
        return JType_CreateJavaBooleanObject(jenv, type, pyArg, objectRef);
    } else if (type == JPy_JChar || type == JPy_JCharacterObj) {
        return JType_CreateJavaCharacterObject(jenv, type, pyArg, objectRef);
    } else if (type == JPy_JByte || type == JPy_JByteObj) {
        return JType_CreateJavaByteObject(jenv, type, pyArg, objectRef);
    } else if (type == JPy_JShort || type == JPy_JShortObj) {
        return JType_CreateJavaShortObject(jenv, type, pyArg, objectRef);
    } else if (type == JPy_JInt || type == JPy_JIntegerObj) {
        return JType_CreateJavaIntegerObject(jenv, type, pyArg, objectRef);
    } else if (type == JPy_JLong || type == JPy_JLongObj) {
        return JType_CreateJavaLongObject(jenv, type, pyArg, objectRef);
    } else if (type == JPy_JFloat || type == JPy_JFloatObj) {
        return JType_CreateJavaFloatObject(jenv, type, pyArg, objectRef);
    } else if (type == JPy_JDouble || type == JPy_JDoubleObj) {
        return JType_CreateJavaDoubleObject(jenv, type, pyArg, objectRef);
    } else if (type == JPy_JObject) {
        if (PyBool_Check(pyArg)) {
            return JType_CreateJavaBooleanObject(jenv, type, pyArg, objectRef);
        } else if (PyLong_Check(pyArg)) {
            return JType_CreateJavaIntegerObject(jenv, type, pyArg, objectRef);
        } else if (PyFloat_Check(pyArg)) {
            return JType_CreateJavaDoubleObject(jenv, type, pyArg, objectRef);
        } else if (PyUnicode_Check(pyArg)) {
            return JPy_AsJString(jenv, pyArg, objectRef);
        }
    } else if (type == JPy_JString) {
        if (PyUnicode_Check(pyArg)) {
            return JPy_AsJString(jenv, pyArg, objectRef);
        }
    }
    return JType_PythonToJavaConversionError(type, pyArg);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// The following functions deal with type creation, initialisation, and resolution.


/**
 * Fill the type __dict__ with our Java class constructors and methods.
 * Constructors will be available using the key named __jinit__.
 * Methods will be available using their method name.
 */
int JType_ResolveType(JNIEnv* jenv, JPy_JType* type)
{
    PyTypeObject* typeObj;

    if (type->isResolved || type->isResolving) {
        return 0;
    }

    type->isResolving = JNI_TRUE;

    typeObj = (PyTypeObject*) type;
    if (typeObj->tp_base != NULL && JType_Check((PyObject*) typeObj->tp_base)) {
        JPy_JType* baseType = (JPy_JType*) typeObj->tp_base;
        if (!baseType->isResolved) {
            if (JType_ResolveType(jenv, baseType) < 0) {
                type->isResolving = JNI_FALSE;
                return -1;
            }
        }
    }

    //printf("JType_ResolveType 1\n");
    if (JType_ProcessClassConstructors(jenv, type) < 0) {
        type->isResolving = JNI_FALSE;
        return -1;
    }

    //printf("JType_ResolveType 2\n");
    if (JType_ProcessClassMethods(jenv, type) < 0) {
        type->isResolving = JNI_FALSE;
        return -1;
    }

    //printf("JType_ResolveType 3\n");
    if (JType_ProcessClassFields(jenv, type) < 0) {
        type->isResolving = JNI_FALSE;
        return -1;
    }

    //printf("JType_ResolveType 4\n");
    type->isResolving = JNI_FALSE;
    type->isResolved = JNI_TRUE;
    return 0;
}

jboolean JType_AcceptMethod(JPy_JType* declaringClass, JPy_JMethod* method)
{
    PyObject* callable;
    PyObject* callableResult;

    //printf("JType_AcceptMethod: javaName='%s'\n", overloadedMethod->declaringClass->javaName);

    callable = PyDict_GetItemString(JPy_Type_Callbacks, declaringClass->javaName);
    if (callable != NULL) {
        if (PyCallable_Check(callable)) {
            callableResult = PyObject_CallFunction(callable, "OO", declaringClass, method);
            if (callableResult == Py_None || callableResult == Py_False) {
                return JNI_FALSE;
            } else if (callableResult == NULL) {
                JPy_DEBUG_PRINTF("JType_AcceptMethod: warning: failed to invoke callback on method addition\n");
                // Ignore this problem and continue
            }
        }
    }

    return JNI_TRUE;
}


int JType_ProcessMethod(JNIEnv* jenv, JPy_JType* type, PyObject* methodKey, const char* methodName, jclass returnType, jarray paramTypes, jboolean isStatic, jmethodID mid)
{
    JPy_ParamDescriptor* paramDescriptors = NULL;
    JPy_ReturnDescriptor* returnDescriptor = NULL;
    jint paramCount;
    JPy_JMethod* method;

    paramCount = (*jenv)->GetArrayLength(jenv, paramTypes);
    JPy_DEBUG_PRINTF("JType_ProcessMethod: methodName=%s, paramCount=%d, isStatic=%d, mid=%p\n", methodName, paramCount, isStatic, mid);
    if (paramCount > 0) {
        paramDescriptors = JType_CreateParamDescriptors(jenv, paramCount, paramTypes);
        if (paramDescriptors == NULL) {
            JPy_DEBUG_PRINTF("JType_ProcessMethod: error: Java method %s rejected because an error occurred during parameter type processing\n", methodName);
            return -1;
        }
    } else {
        paramDescriptors = NULL;
    }

    if (returnType != NULL) {
        returnDescriptor = JType_CreateReturnDescriptor(jenv, returnType);
        if (returnDescriptor == NULL) {
            PyMem_Del(paramDescriptors);
            JPy_DEBUG_PRINTF("JType_ProcessMethod: error: Java method %s rejected because an error occurred during return type processing\n", methodName);
            return -1;
        }
    } else {
        returnDescriptor = NULL;
    }

    method = JMethod_New(methodKey, paramCount, paramDescriptors, returnDescriptor, isStatic, mid);
    if (method == NULL) {
        PyMem_Del(paramDescriptors);
        PyMem_Del(returnDescriptor);
        JPy_DEBUG_PRINTF("JType_ProcessMethod: error: Java method %s rejected because an error occurred during method instantiation\n", methodName);
        return -1;
    }

    if (JType_AcceptMethod(type, method)) {
        JType_InitMethodParamDescriptorFunctions(type, method);
        JType_AddMethod(type, method);
    } else {
        JMethod_Del(method);
    }

    return 0;
}

int JType_InitComponentType(JNIEnv* jenv, JPy_JType* type, jboolean resolve)
{
    jclass componentTypeRef;

    componentTypeRef = (jclass) (*jenv)->CallObjectMethod(jenv, type->classRef, JPy_Class_GetComponentType_MID);
    if (componentTypeRef != NULL) {
        type->componentType = JType_GetType(jenv, componentTypeRef, resolve);
        if (type->componentType == NULL) {
            return -1;
        }
        Py_INCREF(type->componentType);
    } else {
        type->componentType = NULL;
    }

    return 0;
}

int JType_InitSuperType(JNIEnv* jenv, JPy_JType* type, jboolean resolve)
{
    jclass superClassRef;

    superClassRef = (*jenv)->GetSuperclass(jenv, type->classRef);
    if (superClassRef != NULL) {
        type->superType = JType_GetType(jenv, superClassRef, resolve);
        if (type->superType == NULL) {
            return -1;
        }
        Py_INCREF(type->superType);
    } else {
        type->superType = NULL;
    }

    return 0;
}


int JType_ProcessClassConstructors(JNIEnv* jenv, JPy_JType* type)
{
    jclass classRef;
    jobject constructors;
    jobject constructor;
    jobject parameterTypes;
    jint modifiers;
    jint constrCount;
    jint i;
    jboolean isPublic;
    jmethodID mid;
    PyObject* methodKey;

    classRef = type->classRef;
    methodKey = Py_BuildValue("s", JPy_JTYPE_ATTR_NAME_JINIT);
    constructors = (*jenv)->CallObjectMethod(jenv, classRef, JPy_Class_GetDeclaredConstructors_MID);
    constrCount = (*jenv)->GetArrayLength(jenv, constructors);

    JPy_DEBUG_PRINTF("JType_ProcessClassConstructors: constrCount=%d\n", constrCount);

    for (i = 0; i < constrCount; i++) {
        constructor = (*jenv)->GetObjectArrayElement(jenv, constructors, i);
        modifiers = (*jenv)->CallIntMethod(jenv, constructor, JPy_Constructor_GetModifiers_MID);
        isPublic = (modifiers & 0x0001) != 0;
        if (isPublic) {
            parameterTypes = (*jenv)->CallObjectMethod(jenv, constructor, JPy_Constructor_GetParameterTypes_MID);
            mid = (*jenv)->FromReflectedMethod(jenv, constructor);
            JType_ProcessMethod(jenv, type, methodKey, JPy_JTYPE_ATTR_NAME_JINIT, NULL, parameterTypes, 1, mid);
        }
    }
    return 0;
}


int JType_ProcessClassFields(JNIEnv* jenv, JPy_JType* type)
{
    jclass classRef;
    jobject fields;
    jobject field;
    jobject fieldNameStr;
    jobject fieldTypeObj;
    jint modifiers;
    jint fieldCount;
    jint i;
    jboolean isStatic;
    jboolean isPublic;
    jboolean isFinal;
    const char * fieldName;
    jfieldID fid;
    PyObject* fieldKey;

    classRef = type->classRef;

    fields = (*jenv)->CallObjectMethod(jenv, classRef, JPy_Class_GetDeclaredFields_MID);
    fieldCount = (*jenv)->GetArrayLength(jenv, fields);

    JPy_DEBUG_PRINTF("JType_ProcessClassFields: fieldCount=%d\n", fieldCount);

    for (i = 0; i < fieldCount; i++) {
        field = (*jenv)->GetObjectArrayElement(jenv, fields, i);
        modifiers = (*jenv)->CallIntMethod(jenv, field, JPy_Field_GetModifiers_MID);
        // see http://docs.oracle.com/javase/6/docs/api/constant-values.html#java.lang.reflect.Modifier.PUBLIC
        isPublic = (modifiers & 0x0001) != 0;
        isStatic = (modifiers & 0x0008) != 0;
        isFinal  = (modifiers & 0x0010) != 0;
        if (isPublic) {
            fieldNameStr = (*jenv)->CallObjectMethod(jenv, field, JPy_Field_GetName_MID);
            fieldTypeObj = (*jenv)->CallObjectMethod(jenv, field, JPy_Field_GetType_MID);
            fid = (*jenv)->FromReflectedField(jenv, field);

            fieldName = (*jenv)->GetStringUTFChars(jenv, fieldNameStr, NULL);
            fieldKey = Py_BuildValue("s", fieldName);
            JType_ProcessField(jenv, type, fieldKey, fieldName, fieldTypeObj, isStatic, isFinal, fid);

            (*jenv)->ReleaseStringUTFChars(jenv, fieldNameStr, fieldName);
        }
    }
    return 0;
}

int JType_ProcessClassMethods(JNIEnv* jenv, JPy_JType* type)
{
    jclass classRef;
    jobject methods;
    jobject method;
    jobject methodNameStr;
    jobject returnType;
    jobject parameterTypes;
    jint modifiers;
    jint methodCount;
    jint i;
    jboolean isStatic;
    jboolean isPublic;
    const char * methodName;
    jmethodID mid;
    PyObject* methodKey;

    classRef = type->classRef;

    methods = (*jenv)->CallObjectMethod(jenv, classRef, JPy_Class_GetDeclaredMethods_MID);
    methodCount = (*jenv)->GetArrayLength(jenv, methods);

    JPy_DEBUG_PRINTF("JType_ProcessClassMethods: methodCount=%d\n", methodCount);

    for (i = 0; i < methodCount; i++) {
        method = (*jenv)->GetObjectArrayElement(jenv, methods, i);
        modifiers = (*jenv)->CallIntMethod(jenv, method, JPy_Method_GetModifiers_MID);
        // see http://docs.oracle.com/javase/6/docs/api/constant-values.html#java.lang.reflect.Modifier.PUBLIC
        isPublic = (modifiers & 0x0001) != 0;
        isStatic = (modifiers & 0x0008) != 0;
        if (isPublic) {
            methodNameStr = (*jenv)->CallObjectMethod(jenv, method, JPy_Method_GetName_MID);
            returnType = (*jenv)->CallObjectMethod(jenv, method, JPy_Method_GetReturnType_MID);
            parameterTypes = (*jenv)->CallObjectMethod(jenv, method, JPy_Method_GetParameterTypes_MID);
            mid = (*jenv)->FromReflectedMethod(jenv, method);

            methodName = (*jenv)->GetStringUTFChars(jenv, methodNameStr, NULL);
            methodKey = Py_BuildValue("s", methodName);
            JType_ProcessMethod(jenv, type, methodKey, methodName, returnType, parameterTypes, isStatic, mid);

            (*jenv)->ReleaseStringUTFChars(jenv, methodNameStr, methodName);
        }
    }
    return 0;
}

jboolean JType_AcceptField(JPy_JType* declaringClass, JPy_JField* field)
{
    return JNI_TRUE;
}

int JType_AddField(JPy_JType* declaringClass, JPy_JField* field)
{
    PyObject* typeDict;

    typeDict = declaringClass->typeObj.tp_dict;
    if (typeDict == NULL) {
        PyErr_SetString(PyExc_RuntimeError, "internal error: missing attribute '__dict__' in JType");
        return -1;
    }

    PyDict_SetItem(typeDict, field->name, (PyObject*) field);
    return 0;
}

int JType_AddFieldAttribute(JNIEnv* jenv, JPy_JType* declaringClass, PyObject* fieldName, JPy_JType* fieldType, jfieldID fid)
{
    PyObject* typeDict;
    PyObject* fieldValue;

    typeDict = declaringClass->typeObj.tp_dict;
    if (typeDict == NULL) {
        PyErr_SetString(PyExc_RuntimeError, "internal error: missing attribute '__dict__' in JType");
        return -1;
    }

    if (fieldType == JPy_JBoolean) {
        jboolean item = (*jenv)->GetStaticBooleanField(jenv, declaringClass->classRef, fid);
        fieldValue = JPy_FROM_JBOOLEAN(item);
    } else if (fieldType == JPy_JChar) {
        jchar item = (*jenv)->GetStaticCharField(jenv, declaringClass->classRef, fid);
        fieldValue = JPy_FROM_JCHAR(item);
    } else if (fieldType == JPy_JByte) {
        jbyte item = (*jenv)->GetStaticByteField(jenv, declaringClass->classRef, fid);
        fieldValue = JPy_FROM_JBYTE(item);
    } else if (fieldType == JPy_JShort) {
        jshort item = (*jenv)->GetStaticShortField(jenv, declaringClass->classRef, fid);
        fieldValue = JPy_FROM_JSHORT(item);
    } else if (fieldType == JPy_JInt) {
        jint item = (*jenv)->GetStaticIntField(jenv, declaringClass->classRef, fid);
        fieldValue = JPy_FROM_JINT(item);
    } else if (fieldType == JPy_JLong) {
        jlong item = (*jenv)->GetStaticLongField(jenv, declaringClass->classRef, fid);
        fieldValue = JPy_FROM_JLONG(item);
    } else if (fieldType == JPy_JFloat) {
        jfloat item = (*jenv)->GetStaticFloatField(jenv, declaringClass->classRef, fid);
        fieldValue = JPy_FROM_JFLOAT(item);
    } else if (fieldType == JPy_JDouble) {
        jdouble item = (*jenv)->GetStaticDoubleField(jenv, declaringClass->classRef, fid);
        fieldValue = JPy_FROM_JDOUBLE(item);
    } else if (fieldType == JPy_JString) {
        jobject objectRef = (*jenv)->GetStaticObjectField(jenv, declaringClass->classRef, fid);
        fieldValue = JPy_FromJString(jenv, objectRef);
    } else {
        jobject objectRef = (*jenv)->GetStaticObjectField(jenv, declaringClass->classRef, fid);
        fieldValue = JPy_FromJObjectWithType(jenv, objectRef, (JPy_JType*) fieldType);
    }
    PyDict_SetItem(typeDict, fieldName, fieldValue);
    return 0;
}

int JType_ProcessField(JNIEnv* jenv, JPy_JType* declaringClass, PyObject* fieldKey, const char* fieldName, jclass fieldClassRef, jboolean isStatic, jboolean isFinal, jfieldID fid)
{
    JPy_JField* field;
    JPy_JType* fieldType;

    fieldType = JType_GetType(jenv, fieldClassRef, JNI_FALSE);
    if (fieldType == NULL) {
        JPy_DEBUG_PRINTF("JType_ProcessField: error: Java field %s rejected because an error occurred during type processing\n", fieldName);
        return -1;
    }

    if (isStatic && isFinal) {
        // Add static final values to the JPy_JType's tp_dict.
        // todo: Note that this is a workaround only, because the JPy_JType's tp_getattro slot is not called.
        if (JType_AddFieldAttribute(jenv, declaringClass, fieldKey, fieldType, fid) < 0) {
            return -1;
        }
    } else if (!isStatic) {
        // Add instance field accessor to the JPy_JType's tp_dict, this will be evaluated in the JPy_JType's tp_setattro and tp_getattro slots.
        field = JField_New(declaringClass, fieldKey, fieldType, isStatic, isFinal, fid);
        if (field == NULL) {
            JPy_DEBUG_PRINTF("JType_ProcessField: error: Java field %s rejected because an error occurred during field instantiation\n", fieldName);
            return -1;
        }

        if (JType_AcceptField(declaringClass, field)) {
            JType_AddField(declaringClass, field);
        } else {
            JField_Del(field);
        }
    } else {
        JPy_DEBUG_PRINTF("JType_ProcessField: warning: Java field %s rejected because is is static, but not final\n", fieldName);
    }

    return 0;
}


void JType_InitMethodParamDescriptorFunctions(JPy_JType* type, JPy_JMethod* method)
{
    int index;
    for (index = 0; index < method->paramCount; index++) {
        JType_InitParamDescriptorFunctions(method->paramDescriptors + index);
    }
}

int JType_AddMethod(JPy_JType* type, JPy_JMethod* method)
{
    PyObject* typeDict;
    PyObject* methodValue;
    JPy_JOverloadedMethod* overloadedMethod;

    typeDict = type->typeObj.tp_dict;
    if (typeDict == NULL) {
        PyErr_SetString(PyExc_RuntimeError, "internal error: missing attribute '__dict__' in JType");
        return -1;
    }

    methodValue = PyDict_GetItem(typeDict, method->name);
    if (methodValue == NULL) {
        overloadedMethod = JOverloadedMethod_New(type, method->name, method);
        return PyDict_SetItem(typeDict, method->name, (PyObject*) overloadedMethod);
    } else if (PyObject_TypeCheck(methodValue, &JOverloadedMethod_Type)) {
        overloadedMethod = (JPy_JOverloadedMethod*) methodValue;
        return JOverloadedMethod_AddMethod(overloadedMethod, method);
    } else {
        PyErr_SetString(PyExc_RuntimeError, "internal error: expected type 'JOverloadedMethod' in '__dict__' of a JType");
        return -1;
    }
}

/**
 * Returns NULL (error), Py_None (borrowed ref), or a JPy_JOverloadedMethod* (borrowed ref)
 */
PyObject* JType_GetOverloadedMethod(JNIEnv* jenv, JPy_JType* type, PyObject* methodName, jboolean useSuperClass)
{
    PyObject* typeDict;
    PyObject* methodValue;

    typeDict = type->typeObj.tp_dict;
    if (typeDict == NULL) {
        PyErr_SetString(PyExc_RuntimeError, "internal error: missing attribute '__dict__' in JType");
        return NULL;
    }

    methodValue = PyDict_GetItem(typeDict, methodName);
    if (methodValue == NULL) {
        if (useSuperClass && type->superType != NULL) {
            return JType_GetOverloadedMethod(jenv, type->superType, methodName, JNI_TRUE);
        } else {
            return Py_None;
        }
    }

    if (PyObject_TypeCheck(methodValue, &JOverloadedMethod_Type)) {
        return methodValue;
    } else {
        PyErr_SetString(PyExc_RuntimeError, "internal error: expected type 'JOverloadedMethod' in '__dict__' of a JType");
        return NULL;
    }
}

JPy_ReturnDescriptor* JType_CreateReturnDescriptor(JNIEnv* jenv, jclass returnClass)
{
    JPy_ReturnDescriptor* returnDescriptor;
    JPy_JType* type;

    returnDescriptor = PyMem_New(JPy_ReturnDescriptor, 1);
    if (returnDescriptor == NULL) {
        PyErr_NoMemory();
        return NULL;
    }

    type = JType_GetType(jenv, returnClass, JNI_FALSE);
    if (type == NULL) {
        return NULL;
    }

    returnDescriptor->type = type;
    Py_INCREF((PyObject*) type);

    JPy_DEBUG_PRINTF("JType_ProcessReturnType: type->tp_name='%s', type=%p\n", Py_TYPE(type)->tp_name, type);

    return returnDescriptor;
}


JPy_ParamDescriptor* JType_CreateParamDescriptors(JNIEnv* jenv, int paramCount, jarray paramClasses)
{
    JPy_ParamDescriptor* paramDescriptors;
    JPy_ParamDescriptor* paramDescriptor;
    JPy_JType* type;
    jclass paramClass;
    int i;

    paramDescriptors = PyMem_New(JPy_ParamDescriptor, paramCount);
    if (paramDescriptors == NULL) {
        PyErr_NoMemory();
        return NULL;
    }

    for (i = 0; i < paramCount; i++) {
        paramClass = (*jenv)->GetObjectArrayElement(jenv, paramClasses, i);
        paramDescriptor = paramDescriptors + i;

        type = JType_GetType(jenv, paramClass, JNI_FALSE);
        if (type == NULL) {
            return NULL;
        }

        paramDescriptor->type = type;
        Py_INCREF((PyObject*) paramDescriptor->type);
    }

    return paramDescriptors;
}

int JType_MatchPyArgAsJBooleanParam(JNIEnv* jenv, JPy_ParamDescriptor* paramDescriptor, PyObject* pyArg)
{
    if (PyBool_Check(pyArg)) return 100;
    else if (PyLong_Check(pyArg)) return 10;
    else return 0;
}

int JType_ConvertPyArgToJBooleanArg(JNIEnv* jenv, JPy_ParamDescriptor* paramDescriptor, PyObject* pyArg, jvalue* value, JPy_ArgDisposer* disposer)
{
    value->z = JPy_AS_JBOOLEAN(pyArg);
    return 0;
}

int JType_MatchPyArgAsJByteParam(JNIEnv* jenv, JPy_ParamDescriptor* paramDescriptor, PyObject* pyArg)
{
    if (PyLong_Check(pyArg)) return 100;
    else if (PyBool_Check(pyArg)) return 10;
    else return 0;
}

int JType_ConvertPyArgToJByteArg(JNIEnv* jenv, JPy_ParamDescriptor* paramDescriptor, PyObject* pyArg, jvalue* value, JPy_ArgDisposer* disposer)
{
    value->b = JPy_AS_JBYTE(pyArg);
    return 0;
}

int JType_MatchPyArgAsJCharParam(JNIEnv* jenv, JPy_ParamDescriptor* paramDescriptor, PyObject* pyArg)
{
    if (PyLong_Check(pyArg)) return 100;
    else if (PyBool_Check(pyArg)) return 10;
    else return 0;
}

int JType_ConvertPyArgToJCharArg(JNIEnv* jenv, JPy_ParamDescriptor* paramDescriptor, PyObject* pyArg, jvalue* value, JPy_ArgDisposer* disposer)
{
    value->c = JPy_AS_JCHAR(pyArg);
    return 0;
}

int JType_MatchPyArgAsJShortParam(JNIEnv* jenv, JPy_ParamDescriptor* paramDescriptor, PyObject* pyArg)
{
    if (PyLong_Check(pyArg)) return 100;
    else if (PyBool_Check(pyArg)) return 10;
    else return 0;
}

int JType_ConvertPyArgToJShortArg(JNIEnv* jenv, JPy_ParamDescriptor* paramDescriptor, PyObject* pyArg, jvalue* value, JPy_ArgDisposer* disposer)
{
    value->s = JPy_AS_JSHORT(pyArg);
    return 0;
}

int JType_MatchPyArgAsJIntParam(JNIEnv* jenv, JPy_ParamDescriptor* paramDescriptor, PyObject* pyArg)
{
    if (PyLong_Check(pyArg)) return 100;
    else if (PyBool_Check(pyArg)) return 10;
    else return 0;
}

int JType_ConvertPyArgToJIntArg(JNIEnv* jenv, JPy_ParamDescriptor* paramDescriptor, PyObject* pyArg, jvalue* value, JPy_ArgDisposer* disposer)
{
    value->i = JPy_AS_JINT(pyArg);
    return 0;
}

int JType_MatchPyArgAsJLongParam(JNIEnv* jenv, JPy_ParamDescriptor* paramDescriptor, PyObject* pyArg)
{
    if (PyLong_Check(pyArg)) return 100;
    else if (PyBool_Check(pyArg)) return 10;
    else return 0;
}

int JType_ConvertPyArgToJLongArg(JNIEnv* jenv, JPy_ParamDescriptor* paramDescriptor, PyObject* pyArg, jvalue* value, JPy_ArgDisposer* disposer)
{
    value->j = JPy_AS_JLONG(pyArg);
    return 0;
}

int JType_MatchPyArgAsJFloatParam(JNIEnv* jenv, JPy_ParamDescriptor* paramDescriptor, PyObject* pyArg)
{
    if (PyFloat_Check(pyArg)) return 90; // not 100, in order to give 'double' a chance
    else if (PyNumber_Check(pyArg)) return 50;
    else if (PyLong_Check(pyArg)) return 10;
    else if (PyBool_Check(pyArg)) return 1;
    else return 0;
}

int JType_ConvertPyArgToJFloatArg(JNIEnv* jenv, JPy_ParamDescriptor* paramDescriptor, PyObject* pyArg, jvalue* value, JPy_ArgDisposer* disposer)
{
    value->f = JPy_AS_JFLOAT(pyArg);
    return 0;
}

int JType_MatchPyArgAsJDoubleParam(JNIEnv* jenv, JPy_ParamDescriptor* paramDescriptor, PyObject* pyArg)
{
    if (PyFloat_Check(pyArg)) return 100;
    else if (PyNumber_Check(pyArg)) return 50;
    else if (PyLong_Check(pyArg)) return 10;
    else if (PyBool_Check(pyArg)) return 1;
    else return 0;
}

int JType_ConvertPyArgToJDoubleArg(JNIEnv* jenv, JPy_ParamDescriptor* paramDescriptor, PyObject* pyArg, jvalue* value, JPy_ArgDisposer* disposer)
{
    value->d = JPy_AS_JDOUBLE(pyArg);
    return 0;
}

int JType_MatchPyArgAsJStringParam(JNIEnv* jenv, JPy_ParamDescriptor* paramDescriptor, PyObject* pyArg)
{
    if (pyArg == Py_None) {
        // Signal it is possible, but give low priority since we cannot perform any type checks on 'None'
        return 1;
    }
    if (PyUnicode_Check(pyArg)) {
        return 100;
    }
    return 0;
}

int JType_ConvertPyArgToJStringArg(JNIEnv* jenv, JPy_ParamDescriptor* paramDescriptor, PyObject* pyArg, jvalue* value, JPy_ArgDisposer* disposer)
{
    disposer->data = NULL;
    disposer->DisposeArg = JType_DisposeLocalObjectRefArg;
    return JPy_AsJString(jenv, pyArg, &value->l);
}

int JType_MatchPyArgAsJObjectParam(JNIEnv* jenv, JPy_ParamDescriptor* paramDescriptor, PyObject* pyArg)
{
    JPy_JType* paramType;
    JPy_JType* argType;
    JPy_JType* paramComponentType;
    JPy_JType* argComponentType;
    JPy_JObj* argValue;

    if (pyArg == Py_None) {
        // Signal it is possible, but give low priority since we cannot perform any type checks on 'None'
        return 1;
    }

    paramType = paramDescriptor->type;
    paramComponentType = paramType->componentType;

    if (JObj_Check(pyArg)) {
        // pyArg is a Java object

        argType = (JPy_JType*) Py_TYPE(pyArg);
        if (argType == paramType) {
            // pyArg has same type as the parameter
            return 100;
        }

        argValue = (JPy_JObj*) pyArg;
        if ((*jenv)->IsInstanceOf(jenv, argValue->objectRef, paramType->classRef)) {
            argComponentType = argType->componentType;
            if (argComponentType == paramComponentType) {
                // pyArg is an instance of parameter type, and they both have the same component types (which may be null)
                return 90;
            }
            if (argComponentType != NULL && paramComponentType != NULL) {
                // Determines whether an object of clazz1 can be safely cast to clazz2.
                if ((*jenv)->IsAssignableFrom(jenv, argComponentType->classRef, paramComponentType->classRef)) {
                    // pyArg is an instance of parameter array type, and component types are compatible
                    return 80;
                }
            }
        }

        // pyArg type does not match parameter type
        return 0;
    }

    // pyArg is not a Java object

    if (paramComponentType != NULL) {
        // The parameter type is an array type

        if (paramComponentType->isPrimitive && PyObject_CheckBuffer(pyArg)) {
            Py_buffer view;

            // The parameter type is a primitive array type, pyArg is a Python buffer object

            if (PyObject_GetBuffer(pyArg, &view, PyBUF_FORMAT) == 0) {
                JPy_JType* type;
                int matchValue;

                //printf("JType_AssessToJObject: buffer len=%d, itemsize=%d, format=%s\n", view.len, view.itemsize, view.format);

                type = paramComponentType;
                matchValue = 0;
                if (view.format != NULL) {
                    char format = *view.format;
                    if (type == JPy_JBoolean) {
                        matchValue = format == 'b' || format == 'B' ? 100
                                   : view.itemsize == 1 ? 10 : 0;
                    } else if (type == JPy_JByte) {
                        matchValue = format == 'b' ? 100
                                   : format == 'B' ? 90
                                   : view.itemsize == 1 ? 10 : 0;
                    } else if (type == JPy_JChar) {
                        matchValue = format == 'u' ? 100
                                   : format == 'H' ? 90
                                   : format == 'h' ? 80 : view.itemsize == 2 ? 10
                                   : 0;
                    } else if (type == JPy_JShort) {
                        matchValue = format == 'h' ? 100
                                   : format == 'H' ? 90
                                   : view.itemsize == 2 ? 10
                                   : 0;
                    } else if (type == JPy_JInt) {
                        matchValue = format == 'i' || format == 'l' ? 100
                                   : format == 'I' || format == 'L' ? 90
                                   : view.itemsize == 4 ? 10
                                   : 0;
                    } else if (type == JPy_JLong) {
                        matchValue = format == 'q' ? 100
                                   : format == 'Q' ? 90
                                   : view.itemsize == 8 ? 10
                                   : 0;
                    } else if (type == JPy_JFloat) {
                        matchValue = format == 'f' ? 100
                                   : view.itemsize == 4 ? 10 : 0;
                    } else if (type == JPy_JDouble) {
                        matchValue = format == 'd' ? 100
                                   : view.itemsize == 8 ? 10
                                   : 0;
                    }
                } else {
                    if (type == JPy_JBoolean) {
                        matchValue = view.itemsize == 1 ? 10 : 0;
                    } else if (type == JPy_JByte) {
                        matchValue = view.itemsize == 1 ? 10 : 0;
                    } else if (type == JPy_JChar) {
                        matchValue = view.itemsize == 2 ? 10 : 0;
                    } else if (type == JPy_JShort) {
                        matchValue = view.itemsize == 2 ? 10 : 0;
                    } else if (type == JPy_JInt) {
                        matchValue = view.itemsize == 4 ? 10 : 0;
                    } else if (type == JPy_JLong) {
                        matchValue = view.itemsize == 8 ? 10 : 0;
                    } else if (type == JPy_JFloat) {
                        matchValue = view.itemsize == 4 ? 10 : 0;
                    } else if (type == JPy_JDouble) {
                        matchValue = view.itemsize == 8 ? 10 : 0;
                    }
                }

                PyBuffer_Release(&view);
                return matchValue;
            }
        } else if (PySequence_Check(pyArg)) {
            return 10;
        }
    } else if (paramType == JPy_JObject) {
        // Parameter type is not an array type, but any other Java object type
        return 10;
    } else if (paramType == JPy_JBooleanObj) {
        if (PyBool_Check(pyArg)) {
            return 100;
        } else if (PyLong_Check(pyArg)) {
            return 10;
        }
    } else if (paramType == JPy_JCharacterObj
               || paramType == JPy_JByteObj
               || paramType == JPy_JShortObj
               || paramType == JPy_JIntegerObj
               || paramType == JPy_JLongObj) {
        if (PyLong_Check(pyArg)) {
            return 100;
        } else if (PyBool_Check(pyArg)) {
            return 10;
        }
    } else if (paramType == JPy_JFloatObj
               || paramType == JPy_JDoubleObj) {
        if (PyFloat_Check(pyArg)) {
            return 100;
        } else if (PyLong_Check(pyArg)) {
            return 90;
        } else if (PyBool_Check(pyArg)) {
            return 10;
        }
    }

    return 0;
}


int JType_ConvertPyArgToJObjectArg(JNIEnv* jenv, JPy_ParamDescriptor* paramDescriptor, PyObject* pyArg, jvalue* value, JPy_ArgDisposer* disposer)
{
    if (pyArg == Py_None) {
        // Py_None maps to (Java) NULL
        value->l = NULL;
        disposer->data = NULL;
        disposer->DisposeArg = NULL;
    } else if (JObj_Check(pyArg)) {
        // If it is a wrapped Java object, it is always a global reference, so don't dispose it
        JPy_JObj* obj = (JPy_JObj*) pyArg;
        value->l = obj->objectRef;
        disposer->data = NULL;
        disposer->DisposeArg = NULL;
    } else {
        // For any other Python argument, we first check if the formal parameter is a primitive array
        // and the Python argument is a buffer object

        JPy_JType* paramType = paramDescriptor->type;
        JPy_JType* paramComponentType = paramType->componentType;

        if (paramComponentType != NULL && paramComponentType->isPrimitive && PyObject_CheckBuffer(pyArg)) {
            Py_buffer* view;
            int flags;
            Py_ssize_t itemCount;
            jarray array;
            void* arrayItems;
            jint itemSize;
            JPy_JType* type;

            view = PyMem_New(Py_buffer, 1);
            if (view == NULL) {
                PyErr_NoMemory();
                return -1;
            }

            flags = paramDescriptor->isMutable ? PyBUF_WRITABLE : PyBUF_SIMPLE;
            if (PyObject_GetBuffer(pyArg, view, flags) < 0) {
                PyMem_Del(view);
                return -1;
            }

            itemCount = view->len / view->itemsize;
            if (itemCount <= 0) {
                PyBuffer_Release(view);
                PyMem_Del(view);
                PyErr_SetString(PyExc_ValueError, "illegal buffer configuration");
                return -1;
            }

            type = paramComponentType;
            if (type == JPy_JBoolean) {
                array = (*jenv)->NewBooleanArray(jenv, itemCount);
                itemSize = sizeof(jboolean);
            } else if (type == JPy_JByte) {
                array = (*jenv)->NewByteArray(jenv, itemCount);
                itemSize = sizeof(jbyte);
            } else if (type == JPy_JChar) {
                array = (*jenv)->NewCharArray(jenv, itemCount);
                itemSize = sizeof(jchar);
            } else if (type == JPy_JShort) {
                array = (*jenv)->NewShortArray(jenv, itemCount);
                itemSize = sizeof(jshort);
            } else if (type == JPy_JInt) {
                array = (*jenv)->NewIntArray(jenv, itemCount);
                itemSize = sizeof(jint);
            } else if (type == JPy_JLong) {
                array = (*jenv)->NewLongArray(jenv, itemCount);
                itemSize = sizeof(jlong);
            } else if (type == JPy_JFloat) {
                array = (*jenv)->NewFloatArray(jenv, itemCount);
                itemSize = sizeof(jfloat);
            } else if (type == JPy_JDouble) {
                array = (*jenv)->NewDoubleArray(jenv, itemCount);
                itemSize = sizeof(jdouble);
            } else {
                PyBuffer_Release(view);
                PyMem_Del(view);
                PyErr_SetString(PyExc_RuntimeError, "internal error: illegal primitive type");
                return -1;
            }

            if (view->len != itemCount * itemSize) {
                //printf("%ld, %ld, %ld, %ld\n", view->len , view->itemsize, itemCount, itemSize);
                PyBuffer_Release(view);
                PyMem_Del(view);
                PyErr_SetString(PyExc_ValueError, "buffer length is too small");
                return -1;
            }

            if (array == NULL) {
                PyBuffer_Release(view);
                PyMem_Del(view);
                PyErr_NoMemory();
                return -1;
            }

            arrayItems = (*jenv)->GetPrimitiveArrayCritical(jenv, array, NULL);
            if (arrayItems == NULL) {
                PyBuffer_Release(view);
                PyMem_Del(view);
                PyErr_NoMemory();
                return -1;
            }
            memcpy(arrayItems, view->buf, itemCount * itemSize);
            (*jenv)->ReleasePrimitiveArrayCritical(jenv, array, arrayItems, 0);

            value->l = array;
            disposer->data = view;
            disposer->DisposeArg = paramDescriptor->isMutable ? JType_DisposeWritableBufferArg : JType_DisposeReadOnlyBufferArg;
        } else {
            jobject objectRef;
            if (JType_ConvertPythonToJavaObject(jenv, paramType, pyArg, &objectRef) < 0) {
                return -1;
            }
            value->l = objectRef;
            disposer->data = NULL;
            disposer->DisposeArg = JType_DisposeLocalObjectRefArg;
        }
    }

    return 0;
}

int JType_DisposeLocalObjectRefArg(JNIEnv* jenv, jvalue* value, void* data)
{
    if (value->l != NULL) {
        (*jenv)->DeleteLocalRef(jenv, value->l);
    }
    return 0;
}

int JType_DisposeReadOnlyBufferArg(JNIEnv* jenv, jvalue* value, void* data)
{
    Py_buffer* view;
    jarray array;

    array = (jarray) value->l;
    view = (Py_buffer*) data;
    if (array != NULL && view != NULL) {
        (*jenv)->DeleteLocalRef(jenv, array);
        PyBuffer_Release(view);
        PyMem_Del(view);
    }
    return 0;
}

int JType_DisposeWritableBufferArg(JNIEnv* jenv, jvalue* value, void* data)
{
    Py_buffer* view;
    jarray array;
    void* carray;

    array = (jarray) value->l;
    view = (Py_buffer*) data;
    if (array != NULL && view != NULL) {
        // Copy modified array content back into buffer view
        carray = (*jenv)->GetPrimitiveArrayCritical(jenv, array, NULL);
        if (carray != NULL) {
            view = (Py_buffer*) data;
            memcpy(view->buf, carray, view->len);
            (*jenv)->ReleasePrimitiveArrayCritical(jenv, array, carray, 0);
        }
        (*jenv)->DeleteLocalRef(jenv, array);
        PyBuffer_Release(view);
        PyMem_Del(view);
    }
    return 0;
}

void JType_InitParamDescriptorFunctions(JPy_ParamDescriptor* paramDescriptor)
{
    JPy_JType* paramType = paramDescriptor->type;

    if (paramType == JPy_JVoid) {
        paramDescriptor->MatchPyArg = NULL;
        paramDescriptor->ConvertPyArg = NULL;
    } else if (paramType == JPy_JBoolean) {
        paramDescriptor->MatchPyArg = JType_MatchPyArgAsJBooleanParam;
        paramDescriptor->ConvertPyArg = JType_ConvertPyArgToJBooleanArg;
    } else if (paramType == JPy_JByte) {
        paramDescriptor->MatchPyArg = JType_MatchPyArgAsJByteParam;
        paramDescriptor->ConvertPyArg = JType_ConvertPyArgToJByteArg;
    } else if (paramType == JPy_JChar) {
        paramDescriptor->MatchPyArg = JType_MatchPyArgAsJCharParam;
        paramDescriptor->ConvertPyArg = JType_ConvertPyArgToJCharArg;
    } else if (paramType == JPy_JShort) {
        paramDescriptor->MatchPyArg = JType_MatchPyArgAsJShortParam;
        paramDescriptor->ConvertPyArg = JType_ConvertPyArgToJShortArg;
    } else if (paramType == JPy_JInt) {
        paramDescriptor->MatchPyArg = JType_MatchPyArgAsJIntParam;
        paramDescriptor->ConvertPyArg = JType_ConvertPyArgToJIntArg;
    } else if (paramType == JPy_JLong) {
        paramDescriptor->MatchPyArg = JType_MatchPyArgAsJLongParam;
        paramDescriptor->ConvertPyArg = JType_ConvertPyArgToJLongArg;
    } else if (paramType == JPy_JFloat) {
        paramDescriptor->MatchPyArg = JType_MatchPyArgAsJFloatParam;
        paramDescriptor->ConvertPyArg = JType_ConvertPyArgToJFloatArg;
    } else if (paramType == JPy_JDouble) {
        paramDescriptor->MatchPyArg = JType_MatchPyArgAsJDoubleParam;
        paramDescriptor->ConvertPyArg = JType_ConvertPyArgToJDoubleArg;
    } else if (paramType == JPy_JString) {
        paramDescriptor->MatchPyArg = JType_MatchPyArgAsJStringParam;
        paramDescriptor->ConvertPyArg = JType_ConvertPyArgToJStringArg;
    //} else if (paramType == JPy_JMap) {
    //} else if (paramType == JPy_JList) {
    //} else if (paramType == JPy_JSet) {
    } else {
        paramDescriptor->MatchPyArg = JType_MatchPyArgAsJObjectParam;
        paramDescriptor->ConvertPyArg = JType_ConvertPyArgToJObjectArg;
    }
}

/**
 * The JType's tp_repr slot.
 */
PyObject* JType_repr(JPy_JType* self)
{
    printf("JType_repr: self=%p\n", self);
    return PyUnicode_FromFormat("%s(%p)",
                                self->javaName,
                                self->classRef);
}

/**
 * The JType's tp_str slot.
 */
PyObject* JType_str(JPy_JType* self)
{
    JNIEnv* jenv;
    jstring strJObj;
    PyObject* strPyObj;
    jboolean isCopy;
    const char * utfChars;

    JPy_GET_JNI_ENV_OR_RETURN(jenv, NULL)

    printf("JType_str: self=%p\n", self);

    strJObj = (*jenv)->CallObjectMethod(jenv, self->classRef, JPy_Object_ToString_MID);
    utfChars = (*jenv)->GetStringUTFChars(jenv, strJObj, &isCopy);
    strPyObj = PyUnicode_FromFormat("%s", utfChars);
    (*jenv)->ReleaseStringUTFChars(jenv, strJObj, utfChars);
    return strPyObj;
}

/**
 * The JType's tp_dealloc slot.
 */
void JType_dealloc(JPy_JType* self)
{
    JNIEnv* jenv = JPy_GetJNIEnv();

    printf("JType_dealloc: self->javaName='%s', self->classRef=%p\n", self->javaName, self->classRef);

    PyMem_Del(self->javaName);
    self->javaName = NULL;

    if (jenv != NULL && self->classRef != NULL) {
        (*jenv)->DeleteGlobalRef(jenv, self->classRef);
        self->classRef = NULL;
    }

    Py_DECREF(self->superType);
    self->superType = NULL;

    Py_DECREF(self->componentType);
    self->componentType = NULL;

    Py_TYPE(self)->tp_free((PyObject*) self);
}

/**
 * The JType's JType_getattro slot.
 */
PyObject* JType_getattro(JPy_JType* self, PyObject* name)
{
    printf("JType_getattro: %s.%s\n", Py_TYPE(self)->tp_name, PyUnicode_AsUTF8(name));

    if (!self->isResolved && !self->isResolving) {
        JNIEnv* jenv;
        JPy_GET_JNI_ENV_OR_RETURN(jenv, NULL);
        JType_ResolveType(jenv, self);
    }

    return PyObject_GenericGetAttr((PyObject*) self, name);
}


/**
 * The jpy.JType singleton.
 */
PyTypeObject JType_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "jpy.JType",                 /* tp_name */
    sizeof (JPy_JType),          /* tp_basicsize */
    0,                           /* tp_itemsize */
    (destructor) JType_dealloc,  /* tp_dealloc */
    NULL,                         /* tp_print */
    NULL,                         /* tp_getattr */
    NULL,                         /* tp_setattr */
    NULL,                         /* tp_reserved */
    (reprfunc) JType_repr,        /* tp_repr */
    NULL,                         /* tp_as_number */
    NULL,                         /* tp_as_sequence */
    NULL,                         /* tp_as_mapping */
    NULL,                         /* tp_hash  */
    NULL,                         /* tp_call */
    (reprfunc) JType_str,         /* tp_str */
    (getattrofunc)JType_getattro, /* tp_getattro */
    NULL,                         /* tp_setattro */
    NULL,                         /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT|Py_TPFLAGS_BASETYPE,  /* tp_flags */
    "Java Meta Type",             /* tp_doc */
    NULL,                         /* tp_traverse */
    NULL,                         /* tp_clear */
    NULL,                         /* tp_richcompare */
    0,                            /* tp_weaklistoffset */
    NULL,                         /* tp_iter */
    NULL,                         /* tp_iternext */
    NULL,                         /* tp_methods */
    NULL,                         /* tp_members */
    NULL,                         /* tp_getset */
    NULL,                         /* tp_base */
    NULL,                         /* tp_dict */
    NULL,                         /* tp_descr_get */
    NULL,                         /* tp_descr_set */
    0,                            /* tp_dictoffset */
    (initproc) NULL,              /* tp_init */
    NULL,                         /* tp_alloc */
    (newfunc) NULL,               /* tp_new=NULL --> JType instances cannot be created from Python. */
};


