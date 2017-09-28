package org.jpy;

import java.util.*;

/**
 * A simple wrapper around a Python List object that implements a java List of PyObjects.
 */
class PyListWrapper implements List<PyObject> {
    private PyObject pyObject;

    PyListWrapper(PyObject pyObject) {
        this.pyObject = pyObject;
    }

    @Override
    public int size() {
        return pyObject.callMethod("__len__").getIntValue();
    }

    @Override
    public boolean isEmpty() {
        return size() == 0;
    }

    @Override
    public boolean contains(Object o) {
        for (PyObject obj : this) {
            if (obj.equals(o)) {
                return true;
            }
        }
        return false;
    }

    @Override
    public Iterator<PyObject> iterator() {
        return new Iterator<PyObject>() {
            int ii = 0;
            int size = size();

            @Override
            public boolean hasNext() {
                return ii < size;
            }

            @Override
            public PyObject next() {
                return get(ii++);
            }
        };
    }

    @Override
    public PyObject[] toArray() {
        int size = size();

        PyObject [] result = new PyObject[size];
        for (int ii = 0; ii < size; ++ii) {
            result[ii] = get(ii);
        }

        return result;
    }

    @Override
    public <T> T[] toArray(T[] a) {
        int size = size();

        if (a.length < size) {
            a = Arrays.copyOf(a, size);
        }
        for (int ii = 0; ii < size; ++ii) {
            //noinspection unchecked
            a[ii] = (T)get(ii);
        }
        if (a.length > size) {
            a[size] = null;
        }

        return a;
    }

    @Override
    public boolean add(PyObject pyObject) {
        pyObject.callMethod("append", pyObject);
        return true;
    }

    @Override
    public boolean remove(Object o) {
        try {
            pyObject.callMethod("remove", pyObject);
            return true;
        } catch (Exception e) {
            return false;
        }
    }

    @Override
    public boolean containsAll(Collection<?> c) {
        return c.stream().allMatch(this::contains);
    }

    @Override
    public boolean addAll(Collection<? extends PyObject> c) {
        boolean result = false;
        for (PyObject po : c) {
            result |= add(po);
        }
        return result;
    }

    @Override
    public boolean addAll(int index, Collection<? extends PyObject> c) {
        throw new UnsupportedOperationException();
    }

    @Override
    public boolean removeAll(Collection<?> c) {
        throw new UnsupportedOperationException();
    }

    @Override
    public boolean retainAll(Collection<?> c) {
        throw new UnsupportedOperationException();
    }

    @Override
    public void clear() {
        pyObject.callMethod("clear");
    }

    @Override
    public PyObject get(int index) {
        return pyObject.callMethod("__getitem__", index);
    }

    @Override
    public PyObject set(int index, PyObject element) {
        return pyObject.callMethod("__setitem__", index, element);
    }

    @Override
    public void add(int index, PyObject element) {
        pyObject.callMethod("insert", index, element);
    }

    @Override
    public PyObject remove(int index) {
        throw new UnsupportedOperationException();
    }

    @Override
    public int indexOf(Object o) {
        int size = size();

        for (int ii = 0; ii < size; ++ii) {
            PyObject pyObject = get(ii);
            if (pyObject == null ? o == null : pyObject.equals(o)) {
                return ii;
            }
        }

        return -1;
    }

    @Override
    public int lastIndexOf(Object o) {
        int size = size();

        for (int ii = size - 1; ii >= 0; --ii) {
            PyObject pyObject = get(ii);
            if (pyObject == null ? o == null : pyObject.equals(o)) {
                return ii;
            }
        }

        return -1;
    }

    @Override
    public ListIterator<PyObject> listIterator() {
        throw new UnsupportedOperationException();
    }

    @Override
    public ListIterator<PyObject> listIterator(int index) {
        throw new UnsupportedOperationException();
    }

    @Override
    public List<PyObject> subList(int fromIndex, int toIndex) {
        throw new UnsupportedOperationException();
    }
}
