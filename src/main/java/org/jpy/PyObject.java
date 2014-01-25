package org.jpy;

import java.lang.reflect.InvocationHandler;
import java.lang.reflect.Proxy;

import static org.jpy.PyLib.assertPythonRuns;

/**
 * Wraps a Python object (Python/C API type {@code PyObject*}).
 *
 * @author Norman Fomferra
 * @since 1.0
 */
public class PyObject {

    private final long pointer;

    PyObject(long pointer) {
        if (pointer == 0) {
            throw new IllegalArgumentException("pointer == 0");
        }
        PyLib.incRef(pointer);
        this.pointer = pointer;
    }

    @Override
    protected void finalize() throws Throwable {
        super.finalize();
        PyLib.decRef(getPointer());
    }

    public final long getPointer() {
        return pointer;
    }

    public int getIntValue() {
        assertPythonRuns();
        return PyLib.getIntValue(getPointer());
    }

    public double getDoubleValue() {
        assertPythonRuns();
        return PyLib.getDoubleValue(getPointer());
    }

    public String getStringValue() {
        assertPythonRuns();
        return PyLib.getStringValue(getPointer());
    }

    public Object getObjectValue() {
        assertPythonRuns();
        return PyLib.getObjectValue(getPointer());
    }

    public <T> T[] getObjectArrayValue(Class<T> itemType) {
        assertPythonRuns();
        return PyLib.getObjectArrayValue(getPointer(), itemType);
    }

    public PyObject getAttribute(String name) {
        assertPythonRuns();
        long pointer = PyLib.getAttributeObject(getPointer(), name);
        return pointer != 0 ? new PyObject(pointer) : null;
    }

    public <T> T getAttribute(String name, Class<T> valueType) {
        assertPythonRuns();
        return PyLib.getAttributeValue(getPointer(), name, valueType);
    }

    public void setAttribute(String name, Object value) {
        assertPythonRuns();
        PyLib.setAttributeValue(getPointer(), name, value, value != null ? value.getClass() : (Class) null);
    }

    public <T> void setAttribute(String name, T value, Class<T> valueType) {
        assertPythonRuns();
        PyLib.setAttributeValue(getPointer(), name, value, valueType);
    }

    public PyObject callMethod(String name, Object... args) {
        assertPythonRuns();
        long pointer = PyLib.callAndReturnObject(getPointer(), true, name, args.length, args, null);
        return pointer != 0 ? new PyObject(pointer) : null;
    }

    public PyObject call(String name, Object... args) {
        assertPythonRuns();
        long pointer = PyLib.callAndReturnObject(getPointer(), false, name, args.length, args, null);
        return pointer != 0 ? new PyObject(pointer) : null;
    }

    public <T> T createProxy(Class<T> type) {
        assertPythonRuns();
        return (T) createProxy(PyLib.CallableKind.METHOD, type);
    }

    public Object createProxy(PyLib.CallableKind callableKind, Class<?>... types) {
        assertPythonRuns();
        ClassLoader classLoader = Thread.currentThread().getContextClassLoader();
        InvocationHandler invocationHandler = new PyProxyHandler(this, callableKind);
        return Proxy.newProxyInstance(classLoader, types, invocationHandler);
    }

    @Override
    public final String toString() {
        return String.format("%s(pointer=0x%s)", getClass().getSimpleName(), Long.toHexString(pointer));
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
