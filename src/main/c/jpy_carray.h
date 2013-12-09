#ifndef JPY_CARRAY_H
#define JPY_CARRAY_H

#ifdef __cplusplus
extern "C" {
#endif

#include <Python.h>

/*
 * CHECK: the only reason for this module is the missing Python/C API for the inbuilt array type.
 */

/**
 * Prototype for a function that releases the CArray's internal memory.
 */
typedef void (*CArrayFree)(void* array_elems, int array_length);


/**
 * Factory function for a CArray used if the item's memory is already given.
 *
 * Note that this method does not increment the reference counter. You are responsible for
 * calling Py_INCREF(obj) in the returned obj yourself
 * @param format The item type format. Must be one of "b", "B", "h", "H", "i", "I", "l", "L", "f", "d" (see Python struct module).
 */
PyObject* CArray_FromMemory(const char* format, void* items, int length, CArrayFree free_fn);

/**
 * Factory function for a CArray used if only the number of items is known given.
 *
 * Note that this method does not increment the reference counter. You are responsible for
 * calling Py_INCREF(obj) in the returned obj yourself
 * @param format The item type format. Must be one of "b", "B", "h", "H", "i", "I", "l", "L", "f", "d" (see Python struct module).
 */
PyObject* CArray_New(const char* format, int length);

/**
 * Gets the size in bytes of an item in a CArray.
 * @param format The item type format. Must be one of "b", "B", "h", "H", "i", "I", "l", "L", "f", "d" (see Python struct module).
 */
size_t CArray_ItemSize(const char* format);

/**
 * Default implementation for the CArrayFree function.
 */
void CArray_FreeMemory(void* elems, int length);


/**
 * Represents an instance of the CArray_Type class
 */
typedef struct {
    PyObject_HEAD
    /** must be one of "b", "B", "h", "H", "i", "I", "l", "L", "f", "d" (see Python struct module) */
    char format[2];
    int length;
    size_t item_size;
    void* items;
    CArrayFree free_fn;
    size_t num_exports;
} CArray;


// PyAPI_DATA(PyTypeObject) CArray_Type;
extern PyTypeObject CArray_Type;


#ifdef __cplusplus
}  /* extern "C" */
#endif
#endif /* !JPY_CARRAY_H */