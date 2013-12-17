package org.jpy.python;

import java.lang.reflect.InvocationHandler;
import java.lang.reflect.Proxy;

import static org.jpy.python.PyLib.assertLibInitialized;

/**
 * Wraps a CPython (of type <code>PyObject *</code>).
 *
 * @author Norman Fomferra
 */
public class PyObject {
    public static final long NULL_POINTER = 0;
    public static final PyObject NULL = new PyObject();

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
        if (PyLib.isInitialized()) {
            PyLib.decref(getPointer());
        }
    }

    public final long getPointer() {
        return pointer;
    }

    public int getIntValue() {
        assertLibInitialized();
        return PyLib.getIntValue(getPointer());
    }

    public double getDoubleValue() {
        assertLibInitialized();
        return PyLib.getDoubleValue(getPointer());
    }

    public String getStringValue() {
        assertLibInitialized();
        return PyLib.getStringValue(getPointer());
    }

    public Object getObjectValue() {
        assertLibInitialized();
        return PyLib.getObjectValue(getPointer());
    }

    public PyObject getAttributeValue(String name) {
        assertLibInitialized();
        long value = PyLib.getAttributeValue(getPointer(), name);
        return new PyObject(value);
    }

    public void setAttributeValue(String name, Object value) {
        assertLibInitialized();
        PyLib.setAttributeValue(getPointer(), name, value, value != null ? value.getClass() : null);
    }

    public void setAttributeValue(String name, Object value, Class<?> valueType) {
        assertLibInitialized();
        PyLib.setAttributeValue(getPointer(), name, value, valueType);
    }

    public PyObject callMethod(String name, Object... args) {
        assertLibInitialized();
        long pointer = PyLib.call(getPointer(), true, name, args.length, args, null);
        return new PyObject(pointer);
    }

    public PyObject call(String name, Object... args) {
        assertLibInitialized();
        long pointer = PyLib.call(getPointer(), false, name, args.length, args, null);
        return new PyObject(pointer);
    }

    public <T> T cast(Class<T> type) {
        return (T) newProxyInstance(type);
    }

    public Object newProxyInstance(Class<?>... types) {
        ClassLoader classLoader = Thread.currentThread().getContextClassLoader();
        InvocationHandler invocationHandler = createInvocationHandler();
        return Proxy.newProxyInstance(classLoader, types, invocationHandler);
    }

    protected PyInvocationHandler createInvocationHandler() {
        return new PyInvocationHandler(this, true);
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
