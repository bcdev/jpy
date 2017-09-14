package org.jpy;

import java.util.*;

public class PyDictWrapper implements Map<PyObject, PyObject> {
    private PyObject pyObject;

    PyDictWrapper(PyObject pyObject) {
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
    public boolean containsKey(Object key) {
        return pyObject.callMethod("has_key", key).getBooleanValue();
    }

    @Override
    public boolean containsValue(Object value) {
        return pyObject.callMethod("values").asList().contains(value);
    }

    @Override
    public PyObject get(Object key) {
        return pyObject.callMethod("__getitem__", key);
    }

    @Override
    public PyObject put(PyObject key, PyObject value) {
        return pyObject.callMethod("__setitem__", key, value);
    }

    @Override
    public PyObject remove(Object key) {
        PyObject value = get(key);
        if (value.isNone()) {
            return null;
        } else {
            pyObject.callMethod("__delitem__", key);
            return value;
        }
    }

    @Override
    public void putAll(Map<? extends PyObject, ? extends PyObject> m) {
        throw new UnsupportedOperationException();
    }

    @Override
    public void clear() {
        pyObject.callMethod("clear");
    }

    @Override
    public Set<PyObject> keySet() {
        return new LinkedHashSet<>(pyObject.callMethod("keys").asList());
    }

    @Override
    public Collection<PyObject> values() {
        return pyObject.callMethod("values").asList();
    }

    @Override
    public Set<Entry<PyObject, PyObject>> entrySet() {
        return new EntrySet();
    }

    private class EntrySet implements Set<Entry<PyObject, PyObject>> {
        @Override
        public int size() {
            return PyDictWrapper.this.size();
        }

        @Override
        public boolean isEmpty() {
            return size() == 0;
        }

        @Override
        public boolean contains(Object o) {
            return false;
        }

        @Override
        public Iterator<Entry<PyObject, PyObject>> iterator() {
            return new Iterator<Entry<PyObject, PyObject>>() {
                PyObject it = pyObject.callMethod("__iter__");
                PyObject next = prepareNext();

                private PyObject prepareNext() {
                    try {
                        return next = it.callMethod("next");
                    } catch (Exception e) {
                        return next = null;
                    }
                }

                @Override
                public boolean hasNext() {
                    return next != null;
                }

                @Override
                public Entry<PyObject, PyObject> next() {
                    PyObject oldNext = next;
                    prepareNext();
                    return new Entry<PyObject, PyObject>() {

                        @Override
                        public PyObject getKey() {
                            return oldNext;
                        }

                        @Override
                        public PyObject getValue() {
                            return get(oldNext);
                        }

                        @Override
                        public PyObject setValue(PyObject value) {
                            throw new UnsupportedOperationException();
                        }
                    };
                }
            };
        }

        @Override
        public Object[] toArray() {
            throw new UnsupportedOperationException();
        }

        @Override
        public <T> T[] toArray(T[] a) {
            throw new UnsupportedOperationException();
        }

        @Override
        public boolean add(Entry<PyObject, PyObject> pyObjectPyObjectEntry) {
            throw new UnsupportedOperationException();
        }

        @Override
        public boolean remove(Object o) {
            throw new UnsupportedOperationException();
        }

        @Override
        public boolean containsAll(Collection<?> c) {
            throw new UnsupportedOperationException();
        }

        @Override
        public boolean addAll(Collection<? extends Entry<PyObject, PyObject>> c) {
            throw new UnsupportedOperationException();
        }

        @Override
        public boolean retainAll(Collection<?> c) {
            throw new UnsupportedOperationException();
        }

        @Override
        public boolean removeAll(Collection<?> c) {
            throw new UnsupportedOperationException();
        }

        @Override
        public void clear() {
            throw new UnsupportedOperationException();
        }
    }
}
