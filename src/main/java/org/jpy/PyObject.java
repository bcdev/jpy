package org.jpy;

import java.lang.reflect.InvocationHandler;
import java.lang.reflect.Proxy;

import static org.jpy.PyLib.assertInterpreterInitialized;

/**
 * Wraps a Python object (Python/C API type {@code PyObject*}).
 *
 * @author Norman Fomferra
 * @since 1.0
 */
public class PyObject {
    static final long NULL_POINTER = 0;
    static final PyObject NULL = new PyObject();

    private final long pointer;

    private PyObject() {
        this.pointer = NULL_POINTER;
    }

    PyObject(long pointer) {
        if (pointer == NULL_POINTER) {
            throw new NullPointerException();
        }
        this.pointer = pointer;
    }

    @Override
    protected void finalize() throws Throwable {
        super.finalize();
        if (PyLib.isInterpreterInitialized()) {
            PyLib.decref(getPointer());
        }
    }

    public final long getPointer() {
        return pointer;
    }

    public int getIntValue() {
        assertInterpreterInitialized();
        return PyLib.getIntValue(getPointer());
    }

    public double getDoubleValue() {
        assertInterpreterInitialized();
        return PyLib.getDoubleValue(getPointer());
    }

    public String getStringValue() {
        assertInterpreterInitialized();
        return PyLib.getStringValue(getPointer());
    }

    public Object getObjectValue() {
        assertInterpreterInitialized();
        return PyLib.getObjectValue(getPointer());
    }

    public PyObject getAttribute(String name) {
        assertInterpreterInitialized();
        long value = PyLib.getAttributeObject(getPointer(), name);
        return new PyObject(value);
    }

    public <T> T getAttribute(String name, Class<T> valueType) {
        assertInterpreterInitialized();
        return PyLib.getAttributeValue(getPointer(), name, valueType);
    }

    public void setAttribute(String name, Object value) {
        assertInterpreterInitialized();
        PyLib.setAttributeValue(getPointer(), name, value, value != null ? value.getClass() : (Class) null);
    }

    public <T> void setAttribute(String name, T value, Class<T> valueType) {
        assertInterpreterInitialized();
        PyLib.setAttributeValue(getPointer(), name, value, valueType);
    }

    public PyObject callMethod(String name, Object... args) {
        assertInterpreterInitialized();
        long pointer = PyLib.callAndReturnObject(getPointer(), true, name, args.length, args, null);
        return new PyObject(pointer);
    }

    public PyObject call(String name, Object... args) {
        assertInterpreterInitialized();
        long pointer = PyLib.callAndReturnObject(getPointer(), false, name, args.length, args, null);
        return new PyObject(pointer);
    }

    public <T> T createProxy(Class<T> type) {
        assertInterpreterInitialized();
        return (T) createProxy(PyLib.CallableKind.METHOD, type);
    }

    public Object createProxy(PyLib.CallableKind callableKind, Class<?>... types) {
        assertInterpreterInitialized();
        ClassLoader classLoader = Thread.currentThread().getContextClassLoader();
        InvocationHandler invocationHandler = new PyProxyHandler(this, callableKind);
        return Proxy.newProxyInstance(classLoader, types, invocationHandler);
    }

    @Override
    public final String toString() {
        return String.format("%s(pointer=%s)", getClass().getSimpleName(), Long.toHexString(pointer).toUpperCase());
    }

    @Override
    public final boolean equals(Object o) {
        if (this == o) {
            return true;
        }
        if (!(o instanceof PyObject)) {
            return false;
        }
        PyObject pyObject = (PyObject) o;
        return pointer == pyObject.pointer;
    }

    @Override
    public final int hashCode() {
        return (int) (pointer ^ (pointer >>> 32));
    }
}
