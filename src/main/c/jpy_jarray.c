#include "jpy_module.h"
#include "jpy_jarray.h"


#define PRINT_FLAG(F) printf("JArray_getbufferproc: %s = %d\n", #F, (flags & F) != 0);
#define PRINT_MEMB(F, M) printf("JArray_getbufferproc: %s = " ## F ## "\n", #M, M);


/*
 * Implements the getbuffer() method of the <buffer> interface for CArray_Type
 */
int JArray_GetBufferProc(JPy_JArray* self, Py_buffer* view, int flags, jint itemSize, const char* format)
{
    JNIEnv* jenv;
    jint itemCount;
    jint isCopy;
    void* items;

    JPy_GET_JNI_ENV_OR_RETURN(jenv, -1)

    /*
    printf("CArray_getbufferproc\n");
    PRINT_FLAG(PyBUF_ANY_CONTIGUOUS);
    PRINT_FLAG(PyBUF_CONTIG);
    PRINT_FLAG(PyBUF_CONTIG_RO);
    PRINT_FLAG(PyBUF_C_CONTIGUOUS);
    PRINT_FLAG(PyBUF_FORMAT);
    PRINT_FLAG(PyBUF_FULL);
    PRINT_FLAG(PyBUF_FULL_RO);
    PRINT_FLAG(PyBUF_F_CONTIGUOUS);
    PRINT_FLAG(PyBUF_INDIRECT);
    PRINT_FLAG(PyBUF_ND);
    PRINT_FLAG(PyBUF_READ);
    PRINT_FLAG(PyBUF_RECORDS);
    PRINT_FLAG(PyBUF_RECORDS_RO);
    PRINT_FLAG(PyBUF_SIMPLE);
    PRINT_FLAG(PyBUF_STRIDED);
    PRINT_FLAG(PyBUF_STRIDED_RO);
    PRINT_FLAG(PyBUF_STRIDES);
    PRINT_FLAG(PyBUF_WRITE);
    PRINT_FLAG(PyBUF_WRITEABLE);
    */

    itemCount = (*jenv)->GetArrayLength(jenv, self->arrayRef);

    // According to Python documentation,
    // buffer allocation shall be done in the 5 following steps;

    // Step 1/5
    items = (*jenv)->GetPrimitiveArrayCritical(jenv, self->arrayRef, &isCopy);
    if (items == NULL) {
        PyErr_NoMemory();
        return -1;
    }

    // Step 2/5
    view->buf = items;
    view->len = itemCount * itemSize;
    view->itemsize = itemSize;
    view->readonly = (flags & (PyBUF_WRITE | PyBUF_WRITEABLE)) == 0;
    view->ndim = 1;
    view->shape = PyMem_New(Py_ssize_t, 1);
    *view->shape = itemCount;
    view->strides = PyMem_New(Py_ssize_t, 1);
    *view->strides = itemSize;
    view->suboffsets = NULL;
    if ((flags & PyBUF_FORMAT) != 0) {
        view->format = format;
    } else {
        view->format = "B";
    }

    /*
    PRINT_MEMB("%d", view->len);
    PRINT_MEMB("%d", view->ndim);
    PRINT_MEMB("%s", view->format);
    PRINT_MEMB("%d", view->itemsize);
    PRINT_MEMB("%d", view->readonly);
    PRINT_MEMB("%d", view->shape[0]);
    PRINT_MEMB("%d", view->strides[0]);
    */

    // Step 3/5
    self->exportCount++;

    // Step 4/5
    view->obj = (PyObject*) self;
    Py_INCREF(view->obj);

    // Step 5/5
    return ret;
}

/*
 * Implements the releasebuffer() method of the <buffer> interface for CArray_Type
 */
void JArray_releasebufferproc(JPy_JArray* self, Py_buffer* view)
{
    // Step 1
    self->exportCount--;
    // printf("JArray_releasebufferproc: self->exportCount=%d\n", self->exportCount);

    // Step 2
    if (self->exportCount == 0) {
        JNIEnv* jenv = JPy_GetJNIEnv();
        if (jenv != NULL) {
            (*jenv)->ReleasePrimitiveArrayCritical(jenv, self->arrayRef, view->buf, 0);
        }
        view->buf = NULL;
    }

    // todo - check if we must Py_DECREF here
    //Py_DECREF(view->obj);
}

int JArray_getbufferproc_boolean(JPy_CArray* self, Py_buffer* view, int flags)
{
    return JArray_GetBufferProc(self, view, flags, 1, 'B');
}

int JArray_getbufferproc_char(JPy_CArray* self, Py_buffer* view, int flags)
{
    return JArray_GetBufferProc(self, view, flags, 2, 'H');
}

int JArray_getbufferproc_byte(JPy_CArray* self, Py_buffer* view, int flags)
{
    return JArray_GetBufferProc(self, view, flags, 1, 'b');
}

int JArray_getbufferproc_short(JPy_CArray* self, Py_buffer* view, int flags)
{
    return JArray_GetBufferProc(self, view, flags, 2, 'h');
}

int JArray_getbufferproc_int(JPy_CArray* self, Py_buffer* view, int flags)
{
    return JArray_GetBufferProc(self, view, flags, 4, 'i');
}

int JArray_getbufferproc_long(JPy_CArray* self, Py_buffer* view, int flags)
{
    return JArray_GetBufferProc(self, view, flags, 8, 'q');
}

int JArray_getbufferproc_float(JPy_CArray* self, Py_buffer* view, int flags)
{
    return JArray_GetBufferProc(self, view, flags, 4, 'f');
}

int JArray_getbufferproc_double(JPy_CArray* self, Py_buffer* view, int flags)
{
    return JArray_GetBufferProc(self, view, flags, 8, 'd');
}

static PyBufferProcs JArray_as_buffer_boolean = {
    (getbufferproc) JArray_getbufferproc_boolean,
    (releasebufferproc) JArray_releasebufferproc
};

static PyBufferProcs JArray_as_buffer_char = {
    (getbufferproc) JArray_getbufferproc_char,
    (releasebufferproc) JArray_releasebufferproc
};

static PyBufferProcs JArray_as_buffer_byte = {
    (getbufferproc) JArray_getbufferproc_byte,
    (releasebufferproc) JArray_releasebufferproc
};

static PyBufferProcs JArray_as_buffer_short = {
    (getbufferproc) JArray_getbufferproc_short,
    (releasebufferproc) JArray_releasebufferproc
};

static PyBufferProcs JArray_as_buffer_int = {
    (getbufferproc) JArray_getbufferproc_int,
    (releasebufferproc) JArray_releasebufferproc
};

static PyBufferProcs JArray_as_buffer_long = {
    (getbufferproc) JArray_getbufferproc_long,
    (releasebufferproc) JArray_releasebufferproc
};

static PyBufferProcs JArray_as_buffer_float = {
    (getbufferproc) JArray_getbufferproc_float,
    (releasebufferproc) JArray_releasebufferproc
};

static PyBufferProcs JArray_as_buffer_double = {
    (getbufferproc) JArray_getbufferproc_double,
    (releasebufferproc) JArray_releasebufferproc
};
