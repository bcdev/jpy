/*
 * Copyright 2015 Brockmann Consult GmbH
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * This file was modified by Illumon.
 *
 */
package org.jpy;

import java.util.*;

/**
  * A simple wrapper around PyObjects that are actually Python dictionaries, to present the most useful parts of a
  * Map interface.
  */
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
        return PyLib.pyDictContains(pyObject.getPointer(), key, null);
    }

    /**
     * An extension to the Map interface that allows the use of String keys without generating warnings.
     */
    public boolean containsKey(String key) {
        return PyLib.pyDictContains(pyObject.getPointer(), key, String.class);
    }

    public boolean containsKey(PyObject key) {
        return PyLib.pyDictContains(pyObject.getPointer(), key, PyObject.class);
    }

    @Override
    public boolean containsValue(Object value) {
        return values().contains(value);
    }

    @Override
    public PyObject get(Object key) {
        return pyObject.callMethod("__getitem__", key);
    }

    /**
      * An extension to the Map interface that allows the use of String keys without generating warnings.
      */
    public PyObject get(String key) {
        return pyObject.callMethod("__getitem__", key);
    }

    @Override
    public PyObject put(PyObject key, PyObject value) {
        return putObject(key, value);
    }

    /**
      * An extension to the Map interface that allows the use of Object key-values without generating warnings.
      */
    public PyObject putObject(Object key, Object value) {
        return pyObject.callMethod("__setitem__", key, value);
    }

    @Override
    public PyObject remove(Object key) {
        try {
            PyObject value = get(key);
            pyObject.callMethod("__delitem__", key);
            return value;
        } catch (KeyError ke) {
            return null;
        }
    }

    public PyObject remove(String key) {
        return remove((Object)key);
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
        return new LinkedHashSet<>(PyLib.pyDictKeys(pyObject.getPointer()).asList());
    }

    @Override
    public Collection<PyObject> values() {
        return PyLib.pyDictValues(pyObject.getPointer()).asList();
    }

    @Override
    public Set<Entry<PyObject, PyObject>> entrySet() {
        return new EntrySet();
    }

    /**
      * Gets the underlying PyObject.
      *
      * @return the PyObject wrapped by this dictionary.
      */
    public PyObject unwrap() {
        return pyObject;
    }

    /**
      * Gets the underlying pointer for this object.
      *
      * @return the pointer to the underlying Python object wrapped by this dictionary.
      */
    long getPointer() {
        return pyObject.getPointer();
    }

    /**
      * Copy this dictionary into a new dictionary.
      *
      * @return a wrapped copy of this Python dictionary.
      */
    public PyDictWrapper copy() {
        return new PyDictWrapper(PyLib.copyDict(pyObject.getPointer()));
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
            throw new UnsupportedOperationException();
        }

        @Override
        public Iterator<Entry<PyObject, PyObject>> iterator() {
            return new Iterator<Entry<PyObject, PyObject>>() {
                PyModule builtins = PyModule.getBuiltins();
                PyObject it = pyObject.callMethod("__iter__");
                PyObject next = prepareNext();

                private PyObject prepareNext() {
                    try {
                        return next = builtins.call("next", it);
                    } catch (StopIteration e) {
                        return next = null;
                    }
                }

                @Override
                public boolean hasNext() {
                    return next != null;
                }

                @Override
                public Entry<PyObject, PyObject> next() {
                    final PyObject oldNext = next;
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
