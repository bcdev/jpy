#include "jpy_module.h"
#include "jpy_jtype.h"
#include "jpy_jmethod.h"
#include "jpy_jfield.h"
#include "jpy_jobj.h"
#include "jpy_carray.h"

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


PyTypeObject* JType_GetTypeForName(const char* typeName, jboolean resolve)
{
    JNIEnv* jenv;
    const char* resourceName;
    jclass classRef;

    JPY_GET_JENV(jenv, NULL)

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

    if (JPy_IsDebug()) printf("JType_GetTypeForName: typeName='%s', resourceName='%s'\n", typeName, resourceName);

    classRef = (*jenv)->FindClass(jenv, resourceName);

    if (typeName != resourceName) {
        PyMem_Del((char*) resourceName);
    }

    if (classRef == NULL) {
        PyErr_SetString(PyExc_ValueError, "Java class not found");
        return NULL;
    }

    return JType_GetType(classRef, resolve);
}

/**
 * Returns a new reference.
 */
PyTypeObject* JType_GetType(jclass classRef, jboolean resolve)
{
    JNIEnv* jenv;
    PyObject* typeKey;
    JPy_JType* type;

    JPY_GET_JENV(jenv, NULL)

    typeKey = JPy_GetTypeNameString(jenv, classRef);

    // todo: add check, because the following is a dangerous cast: someone else could have put something else into JPy_Types
    type = (JPy_JType*) PyDict_GetItem(JPy_Types, typeKey);
    if (type == NULL) {

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
            if (JPy_IsDebug()) printf("JType_GetType: error: JType_InitSlots() failed for javaName='%s'\n", type->javaName);

            PyDict_DelItem(JPy_Types, typeKey);

            printf("JType_GetType: after PyDict_DelItem\n");

            return NULL;
        }

    } else {
        Py_DECREF(typeKey);
    }

    if (JPy_IsDebug()) printf("JType_GetType: javaName='%s', resolve=%d, resolved=%d, type=%p\n", type->javaName, resolve, type->isResolved, type);

    if (!type->isResolved && resolve) {
        if (JType_ResolveType(jenv, type) < 0) {
            return NULL;
        }
    }

    return (PyTypeObject*) type;
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

    type->javaName = JPy_AllocTypeNameUTF(jenv, classRef);
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

    if (JPy_IsDebug()) printf("JType_New: javaName='%s', resolve=%d, type=%p\n", type->javaName, resolve, type);

    return type;
}

PyObject* JType_ConvertJavaToPythonObject(JNIEnv* jenv, JPy_JType* type, jobject objectRef)
{
    if (objectRef == NULL) {
        return Py_BuildValue("");
    }

    if (type->componentType == NULL) {
        // todo: check for Java wrapper types as well: java.lang.Byte, java.lang.Short, java.lang.Integer, ...
        if ((PyTypeObject*) type == JPy_JString) {
            return JPy_ConvertJavaToPythonString(jenv, objectRef);
            //return JPy_ConvertJavaToStringToPythonString(jenv, objectRef);
        } else {
            return (PyObject*) JObj_FromType(jenv, type, objectRef);
        }
    } else if (type->componentType->isPrimitive) {
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
        if ((PyTypeObject*) type->componentType == JPy_JBoolean) {
            format = "b";
        } else if ((PyTypeObject*) type->componentType == JPy_JByte) {
            format = "b";
        } else if ((PyTypeObject*) type->componentType == JPy_JChar) {
            format = "H";
        } else if ((PyTypeObject*) type->componentType == JPy_JShort) {
            format = "h";
        } else if ((PyTypeObject*) type->componentType == JPy_JInt) {
            format = "l";
        } else if ((PyTypeObject*) type->componentType == JPy_JLong) {
            format = "q";
        } else if ((PyTypeObject*) type->componentType == JPy_JFloat) {
            format = "f";
        } else if ((PyTypeObject*) type->componentType == JPy_JDouble) {
            format = "d";
        } else {
            // todo: raise internal error
        }

        array = (JPy_CArray*) CArray_New(format, length);
        if (array != NULL) {
            memcpy(array->items, items, array->itemSize * length);
        }

        (*jenv)->ReleasePrimitiveArrayCritical(jenv, objectRef, items, 0);

        return (PyObject*) array;
    } else {
        return (PyObject*) JObj_FromType(jenv, type, objectRef);
    }
}

int JType_ConvertPythonToJavaObject(JNIEnv* jenv, JPy_JType* type, PyObject* arg, jobject* objectRef)
{
    if (arg == Py_None) {
        *objectRef = NULL;
        return 0;
    }
    if (JObj_Check(arg)) {
        *objectRef = ((JPy_JObj*) arg)->objectRef;
        return 0;
    }
    if ((PyTypeObject*) type == JPy_JString && PyUnicode_Check(arg)) {
        // todo: problem of memory leak here: '**objectRef' escapes but we must actually must call jenv->DeleteLocalRef(*objectRef) some time later
        if (JPy_ConvertPythonToJavaString(jenv, arg, objectRef) < 0) {
            return -1;
        }
        return 0;
    }
    PyErr_SetString(PyExc_RuntimeError, "failed to convert Python to Java object");
    return -1;
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
                if (JPy_IsDebug()) printf("JType_AcceptMethod: warning: failed to invoke callback on method addition\n");
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
    if (JPy_IsDebug()) printf("JType_ProcessMethod: methodName=%s, paramCount=%d, isStatic=%d, mid=%p\n", methodName, paramCount, isStatic, mid);
    if (paramCount > 0) {
        paramDescriptors = JType_CreateParamDescriptors(jenv, paramCount, paramTypes);
        if (paramDescriptors == NULL) {
            // todo: log problem
            if (JPy_IsDebug()) printf("JType_ProcessMethod: error: Java method %s rejected because an error occurred during parameter type processing\n", methodName);
            return -1;
        }
    } else {
        paramDescriptors = NULL;
    }

    if (returnType != NULL) {
        returnDescriptor = JType_CreateReturnDescriptor(jenv, returnType);
        if (returnDescriptor == NULL) {
            PyMem_Del(paramDescriptors);
            // todo: log problem
            if (JPy_IsDebug()) printf("JType_ProcessMethod: error: Java method %s rejected because an error occurred during return type processing\n", methodName);
            return -1;
        }
    } else {
        returnDescriptor = NULL;
    }

    method = JMethod_New(methodKey, paramCount, paramDescriptors, returnDescriptor, isStatic, mid);
    if (method == NULL) {
        PyMem_Del(paramDescriptors);
        PyMem_Del(returnDescriptor);
        // todo: log problem
        if (JPy_IsDebug()) printf("JType_ProcessMethod: error: Java method %s rejected because an error occurred during method instantiation\n", methodName);
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
        type->componentType = (JPy_JType*) JType_GetType(componentTypeRef, resolve);
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
        type->superType = (JPy_JType*) JType_GetType(superClassRef, resolve);
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
    methodKey = Py_BuildValue("s", JPy_JINIT_ATTR_NAME);
    constructors = (*jenv)->CallObjectMethod(jenv, classRef, JPy_Class_GetDeclaredConstructors_MID);
    constrCount = (*jenv)->GetArrayLength(jenv, constructors);

    if (JPy_IsDebug()) printf("JType_ProcessClassConstructors: constrCount=%d\n", constrCount);

    for (i = 0; i < constrCount; i++) {
        constructor = (*jenv)->GetObjectArrayElement(jenv, constructors, i);
        modifiers = (*jenv)->CallIntMethod(jenv, constructor, JPy_Constructor_GetModifiers_MID);
        isPublic = (modifiers & 0x0001) != 0;
        if (isPublic) {
            parameterTypes = (*jenv)->CallObjectMethod(jenv, constructor, JPy_Constructor_GetParameterTypes_MID);
            mid = (*jenv)->FromReflectedMethod(jenv, constructor);
            JType_ProcessMethod(jenv, type, methodKey, JPy_JINIT_ATTR_NAME, NULL, parameterTypes, 1, mid);
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

    if (JPy_IsDebug()) printf("JType_ProcessClassFields: fieldCount=%d\n", fieldCount);

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

    if (JPy_IsDebug()) printf("JType_ProcessClassMethods: methodCount=%d\n", methodCount);

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

int JType_AddFieldAttribute(JPy_JType* declaringClass, PyObject* fieldName, PyTypeObject* fieldType, jfieldID fid)
{
    PyObject* typeDict;
    PyObject* fieldValue;
    JNIEnv* jenv;

    typeDict = declaringClass->typeObj.tp_dict;
    if (typeDict == NULL) {
        PyErr_SetString(PyExc_RuntimeError, "internal error: missing attribute '__dict__' in JType");
        return -1;
    }

    JPY_GET_JENV(jenv, -1)

    if (fieldType == JPy_JBoolean) {
        jboolean item = (*jenv)->GetStaticBooleanField(jenv, declaringClass->classRef, fid);
        fieldValue = PyBool_FromLong(item);
    } else if (fieldType == JPy_JByte) {
        jbyte item = (*jenv)->GetStaticByteField(jenv, declaringClass->classRef, fid);
        fieldValue = PyLong_FromLong(item);
    } else if (fieldType == JPy_JChar) {
        jchar item = (*jenv)->GetStaticCharField(jenv, declaringClass->classRef, fid);
        fieldValue = PyLong_FromLong(item);
    } else if (fieldType == JPy_JShort) {
        jshort item = (*jenv)->GetStaticShortField(jenv, declaringClass->classRef, fid);
        fieldValue = PyLong_FromLong(item);
    } else if (fieldType == JPy_JInt) {
        jint item = (*jenv)->GetStaticIntField(jenv, declaringClass->classRef, fid);
        fieldValue = PyLong_FromLong(item);
    } else if (fieldType == JPy_JLong) {
        jlong item = (*jenv)->GetStaticLongField(jenv, declaringClass->classRef, fid);
        fieldValue = PyLong_FromLongLong(item);
    } else if (fieldType == JPy_JFloat) {
        jfloat item = (*jenv)->GetStaticFloatField(jenv, declaringClass->classRef, fid);
        fieldValue = PyFloat_FromDouble(item);
    } else if (fieldType == JPy_JDouble) {
        jdouble item = (*jenv)->GetStaticDoubleField(jenv, declaringClass->classRef, fid);
        fieldValue = PyFloat_FromDouble(item);
    } else {
        jobject objectRef = (*jenv)->GetStaticObjectField(jenv, declaringClass->classRef, fid);
        fieldValue = JType_ConvertJavaToPythonObject(jenv, (JPy_JType*) fieldType, objectRef);
    }
    PyDict_SetItem(typeDict, fieldName, fieldValue);
    return 0;
}

int JType_ProcessField(JNIEnv* jenv, JPy_JType* declaringClass, PyObject* fieldKey, const char* fieldName, jclass fieldClassRef, jboolean isStatic, jboolean isFinal, jfieldID fid)
{
    JPy_JField* field;
    JPy_JType* fieldType;

    fieldType = (JPy_JType*) JType_GetType(fieldClassRef, JNI_FALSE);
    if (fieldType == NULL) {
        if (JPy_IsDebug()) printf("JType_ProcessField: error: Java field %s rejected because an error occurred during type processing\n", fieldName);
        return -1;
    }

    if (isStatic && isFinal) {
        // Add static final values to the JPy_JType's tp_dict.
        // todo: Note that this is a workaround only, because the JPy_JType's tp_getattro slot is not called.
        if (JType_AddFieldAttribute(declaringClass, fieldKey, (PyTypeObject*) fieldType, fid) < 0) {
            return -1;
        }
    } else if (!isStatic) {
        // Add instance field accessor to the JPy_JType's tp_dict, this will be evaluated in the JPy_JType's tp_setattro and tp_getattro slots.
        field = JField_New(declaringClass, fieldKey, fieldType, isStatic, isFinal, fid);
        if (field == NULL) {
            if (JPy_IsDebug()) printf("JType_ProcessField: error: Java field %s rejected because an error occurred during field instantiation\n", fieldName);
            return -1;
        }

        if (JType_AcceptField(declaringClass, field)) {
            JType_AddField(declaringClass, field);
        } else {
            JField_Del(field);
        }
    } else {
        if (JPy_IsDebug()) printf("JType_ProcessField: warning: Java field %s rejected because is is static, but not final\n", fieldName);
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
PyObject* JType_GetOverloadedMethod(JPy_JType* type, PyObject* methodName, jboolean useSuperClass)
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
            return JType_GetOverloadedMethod(type->superType, methodName, JNI_TRUE);
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
    PyTypeObject* type;

    returnDescriptor = PyMem_New(JPy_ReturnDescriptor, 1);
    if (returnDescriptor == NULL) {
        PyErr_NoMemory();
        return NULL;
    }

    type = JType_GetType(returnClass, JNI_FALSE);
    if (type == NULL) {
        return NULL;
    }

    returnDescriptor->type = (JPy_JType*) type;
    Py_INCREF((PyObject*) type);

    // if (JPy_IsDebug()) printf("JType_ProcessReturnType: type->tp_name='%s',
    //                           type=%p\n", type->tp_name, type);

    return returnDescriptor;
}


JPy_ParamDescriptor* JType_CreateParamDescriptors(JNIEnv* jenv, int paramCount, jarray paramClasses)
{
    JPy_ParamDescriptor* paramDescriptors;
    JPy_ParamDescriptor* paramDescriptor;
    PyTypeObject* type;
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

        type = JType_GetType(paramClass, JNI_FALSE);
        if (type == NULL) {
            return NULL;
        }

        paramDescriptor->type = (JPy_JType*) type;
        Py_INCREF((PyObject*) paramDescriptor->type);
    }

    return paramDescriptors;
}

int JType_AssessToJBoolean(JPy_ParamDescriptor* paramDescriptor, PyObject* arg)
{
    if (PyBool_Check(arg)) return 100;
    else if (PyLong_Check(arg)) return 10;
    else return 0;
}

int JType_ConvertToJBoolean(JPy_ParamDescriptor* paramDescriptor, PyObject* arg, jvalue* value, JPy_ArgDisposer* disposer)
{
    value->z = (jboolean) (PyLong_AsLong(arg) != 0);
    return 0;
}

int JType_AssessToJByte(JPy_ParamDescriptor* paramDescriptor, PyObject* arg)
{
    if (PyLong_Check(arg)) return 100;
    else if (PyBool_Check(arg)) return 10;
    else return 0;
}

int JType_ConvertToJByte(JPy_ParamDescriptor* paramDescriptor, PyObject* arg, jvalue* value, JPy_ArgDisposer* disposer)
{
    value->b = (jbyte) PyLong_AsLong(arg);
    return 0;
}

int JType_AssessToJChar(JPy_ParamDescriptor* paramDescriptor, PyObject* arg)
{
    if (PyLong_Check(arg)) return 100;
    else if (PyBool_Check(arg)) return 10;
    else return 0;
}

int JType_ConvertToJChar(JPy_ParamDescriptor* paramDescriptor, PyObject* arg, jvalue* value, JPy_ArgDisposer* disposer)
{
    value->c = (jchar) PyLong_AsLong(arg);
    return 0;
}

int JType_AssessToJShort(JPy_ParamDescriptor* paramDescriptor, PyObject* arg)
{
    if (PyLong_Check(arg)) return 100;
    else if (PyBool_Check(arg)) return 10;
    else return 0;
}

int JType_ConvertToJShort(JPy_ParamDescriptor* paramDescriptor, PyObject* arg, jvalue* value, JPy_ArgDisposer* disposer)
{
    value->s = (jshort) PyLong_AsLong(arg);
    return 0;
}

int JType_AssessToJInt(JPy_ParamDescriptor* paramDescriptor, PyObject* arg)
{
    if (PyLong_Check(arg)) return 100;
    else if (PyBool_Check(arg)) return 10;
    else return 0;
}

int JType_ConvertToJInt(JPy_ParamDescriptor* paramDescriptor, PyObject* arg, jvalue* value, JPy_ArgDisposer* disposer)
{
    value->i = (jint) PyLong_AsLong(arg);
    return 0;
}

int JType_AssessToJLong(JPy_ParamDescriptor* paramDescriptor, PyObject* arg)
{
    if (PyLong_Check(arg)) return 100;
    else if (PyBool_Check(arg)) return 10;
    else return 0;
}

int JType_ConvertToJLong(JPy_ParamDescriptor* paramDescriptor, PyObject* arg, jvalue* value, JPy_ArgDisposer* disposer)
{
    value->j = (jlong) PyLong_AsLongLong(arg);
    return 0;
}

int JType_AssessToJFloat(JPy_ParamDescriptor* paramDescriptor, PyObject* arg)
{
    if (PyFloat_Check(arg)) return 90; // not 100, in order to give 'double' a chance
    else if (PyNumber_Check(arg)) return 50;
    else if (PyLong_Check(arg)) return 10;
    else if (PyBool_Check(arg)) return 1;
    else return 0;
}

int JType_ConvertToJFloat(JPy_ParamDescriptor* paramDescriptor, PyObject* arg, jvalue* value, JPy_ArgDisposer* disposer)
{
    value->f = (jfloat) PyFloat_AsDouble(arg);
    return 0;
}

int JType_AssessToJDouble(JPy_ParamDescriptor* paramDescriptor, PyObject* arg)
{
    if (PyFloat_Check(arg)) return 100;
    else if (PyNumber_Check(arg)) return 50;
    else if (PyLong_Check(arg)) return 10;
    else if (PyBool_Check(arg)) return 1;
    else return 0;
}

int JType_ConvertToJDouble(JPy_ParamDescriptor* paramDescriptor, PyObject* arg, jvalue* value, JPy_ArgDisposer* disposer)
{
    value->d = (jdouble) PyFloat_AsDouble(arg);
    return 0;
}

int JType_AssessToJString(JPy_ParamDescriptor* paramDescriptor, PyObject* arg)
{
    if (arg == Py_None) {
        // Signal it is possible, but give low priority since we cannot perform any type checks on 'None'
        return 1;
    }
    if (PyUnicode_Check(arg)) {
        return 100;
    }
    return 0;
}

int JType_DisposeLocalObjectRef(jvalue* value, void* data)
{
    JNIEnv* jenv;
    JPY_GET_JENV(jenv, -1);
    if (value->l != NULL) {
        (*jenv)->DeleteLocalRef(jenv, value->l);
    }
    return 0;
}

int JType_ConvertToJString(JPy_ParamDescriptor* paramDescriptor, PyObject* arg, jvalue* value, JPy_ArgDisposer* disposer)
{
    JNIEnv* jenv;
    JPY_GET_JENV(jenv, -1);
    disposer->data = NULL;
    disposer->disposeArg = JType_DisposeLocalObjectRef;
    return JPy_ConvertPythonToJavaString(jenv, arg, &value->l);
}

int JType_AssessToJObject(JPy_ParamDescriptor* paramDescriptor, PyObject* arg)
{
    JNIEnv* jenv;
    JPy_JType* paramType;
    JPy_JType* argType;
    JPy_JType* paramComponentType;
    JPy_JType* argComponentType;
    JPy_JObj* argValue;

    if (arg == Py_None) {
        // Signal it is possible, but give low priority since we cannot perform any type checks on 'None'
        return 1;
    }

    paramType = paramDescriptor->type;
    paramComponentType = paramType->componentType;

    if (!JObj_Check(arg)) {
        if (paramComponentType != NULL && paramComponentType->isPrimitive && PyObject_CheckBuffer(arg)) {
            Py_buffer view;

            if (PyObject_GetBuffer(arg, &view, PyBUF_FORMAT) == 0) {
                PyTypeObject* type;
                int matchValue;

                //printf("buffer len=%d, itemsize=%d, format=%s\n", view.len, view.itemsize, view.format);

                type = (PyTypeObject*) paramComponentType;
                matchValue = 0;
                if (view.format == NULL) {
                    char format = *view.format;
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
                } else {
                    char format = *view.format;
                    if (type == JPy_JBoolean) {
                        matchValue = format == 'b' || format == 'B' ? 100 : 0;
                    } else if (type == JPy_JByte) {
                        matchValue = format == 'b' ? 100 : format == 'B' ? 90 : 0;
                    } else if (type == JPy_JChar) {
                        matchValue = format == 'u' ? 100 : format == 'H' ? 90 : format == 'h' ? 80 : 0;
                    } else if (type == JPy_JShort) {
                        matchValue = format == format == 'h' ? 100 : format == 'H' ? 90 : 0;
                    } else if (type == JPy_JInt) {
                        matchValue = format == format == 'i' || format == 'l' ? 100 : format == 'I' || format == 'L' ? 90 : 0;
                    } else if (type == JPy_JLong) {
                        matchValue = format == 'q' ? 100 : format == 'Q' ? 90 : 0;
                    } else if (type == JPy_JFloat) {
                        matchValue = format == 'f' ? 100 : 0;
                    } else if (type == JPy_JDouble) {
                        matchValue = format == 'd' ? 100 : 0;
                    }
                }

                PyBuffer_Release(&view);
                return matchValue;
            }
        }
        return 0;
    }

    argType = (JPy_JType*) Py_TYPE(arg);
    if (argType == paramType) {
        return 100;
    }

    JPY_GET_JENV(jenv, 0)

    argValue = (JPy_JObj*) arg;
    if ((*jenv)->IsInstanceOf(jenv, argValue->objectRef, paramType->classRef)) {
        argComponentType = argType->componentType;
        if (argComponentType == paramComponentType) {
            return 90;
        }
        if (argComponentType != NULL && paramComponentType != NULL) {
            // Determines whether an object of clazz1 can be safely cast to clazz2.
            if ((*jenv)->IsAssignableFrom(jenv, argComponentType->classRef, paramComponentType->classRef)) {
                return 80;
            }
        }
    }

    return 0;
}

int JType_DisposeReadOnlyBuffer(jvalue* value, void* data)
{
    Py_buffer* view;
    JNIEnv* jenv;
    jarray array;

    JPY_GET_JENV(jenv, -1);

    array = (jarray) value->l;
    view = (Py_buffer*) data;
    if (array != NULL && view != NULL) {
        (*jenv)->DeleteLocalRef(jenv, array);
        PyBuffer_Release(view);
        PyMem_Del(view);
    }
    return 0;
}

int JType_DisposeWritableBuffer(jvalue* value, void* data)
{
    Py_buffer* view;
    JNIEnv* jenv;
    jarray array;
    void* carray;

    JPY_GET_JENV(jenv, -1);

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


int JType_ConvertToJObject(JPy_ParamDescriptor* paramDescriptor, PyObject* arg, jvalue* value, JPy_ArgDisposer* disposer)
{
    JNIEnv* jenv;
    JPy_JType* paramType;
    JPy_JType* componentType;

    if (arg == Py_None) {
        value->l = NULL;
        return 0;
    }

    paramType = paramDescriptor->type;
    componentType = paramType->componentType;
    if (componentType != NULL && componentType->isPrimitive && PyObject_CheckBuffer(arg)) {
        Py_buffer* view;
        int flags;
        int itemCount;
        jarray array;
        void* carray;
        int itemSize;
        PyTypeObject* type;

        view = PyMem_New(Py_buffer, 1);
        if (view == NULL) {
            PyErr_NoMemory();
            return -1;
        }

        flags = paramDescriptor->isMutable ? PyBUF_WRITABLE : PyBUF_SIMPLE;
        if (PyObject_GetBuffer(arg, view, flags) < 0) {
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

        JPY_GET_JENV(jenv, 0)

        type = (PyTypeObject*) componentType;
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
            printf("%d, %d, %d, %d\n",view->len , view->itemsize , itemCount , itemSize);
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

        carray = (*jenv)->GetPrimitiveArrayCritical(jenv, array, NULL);
        if (carray == NULL) {
            PyBuffer_Release(view);
            PyMem_Del(view);
            PyErr_NoMemory();
            return -1;
        }
        memcpy(carray, view->buf, itemCount * itemSize);
        (*jenv)->ReleasePrimitiveArrayCritical(jenv, array, carray, 0);

        value->l = array;
        disposer->data = view;
        disposer->disposeArg = paramDescriptor->isMutable ? JType_DisposeWritableBuffer : JType_DisposeReadOnlyBuffer;
    } else {
        JPy_JObj* obj = (JPy_JObj*) arg;
        value->l = obj->objectRef;
        disposer->data = NULL;
        disposer->disposeArg = NULL;
    }
    return 0;
}

void JType_InitParamDescriptorFunctions(JPy_ParamDescriptor* paramDescriptor)
{
    PyTypeObject* paramType = (PyTypeObject*) paramDescriptor->type;

    if (paramType == JPy_JVoid) {
        paramDescriptor->paramAssessor = NULL;
        paramDescriptor->paramConverter = NULL;
    } else if (paramType == JPy_JBoolean) {
        paramDescriptor->paramAssessor = JType_AssessToJBoolean;
        paramDescriptor->paramConverter = JType_ConvertToJBoolean;
    } else if (paramType == JPy_JByte) {
        paramDescriptor->paramAssessor = JType_AssessToJByte;
        paramDescriptor->paramConverter = JType_ConvertToJByte;
    } else if (paramType == JPy_JChar) {
        paramDescriptor->paramAssessor = JType_AssessToJChar;
        paramDescriptor->paramConverter = JType_ConvertToJChar;
    } else if (paramType == JPy_JShort) {
        paramDescriptor->paramAssessor = JType_AssessToJShort;
        paramDescriptor->paramConverter = JType_ConvertToJShort;
    } else if (paramType == JPy_JInt) {
        paramDescriptor->paramAssessor = JType_AssessToJInt;
        paramDescriptor->paramConverter = JType_ConvertToJInt;
    } else if (paramType == JPy_JLong) {
        paramDescriptor->paramAssessor = JType_AssessToJLong;
        paramDescriptor->paramConverter = JType_ConvertToJLong;
    } else if (paramType == JPy_JFloat) {
        paramDescriptor->paramAssessor = JType_AssessToJFloat;
        paramDescriptor->paramConverter = JType_ConvertToJFloat;
    } else if (paramType == JPy_JDouble) {
        paramDescriptor->paramAssessor = JType_AssessToJDouble;
        paramDescriptor->paramConverter = JType_ConvertToJDouble;
    } else if (paramType == JPy_JString) {
        paramDescriptor->paramAssessor = JType_AssessToJString;
        paramDescriptor->paramConverter = JType_ConvertToJString;
    //} else if (paramType == JPy_JMap) {
    //} else if (paramType == JPy_JList) {
    //} else if (paramType == JPy_JSet) {
    } else {
        // todo: use paramDescriptor->is_mutable / is_return to select more specific functions
        paramDescriptor->paramAssessor = JType_AssessToJObject;
        paramDescriptor->paramConverter = JType_ConvertToJObject;
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

    JPY_GET_JENV(jenv, NULL)

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
        JPY_GET_JENV(jenv, NULL);
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


