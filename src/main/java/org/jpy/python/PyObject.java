package org.jpy.python;

import java.lang.reflect.InvocationHandler;
import java.lang.reflect.Proxy;

import static org.jpy.python.PyLib.assertLibInitialized;

/**
 * Wraps a CPython (of type <code>PyObject *</code>).
 *
 * <p/>
 * <i>Neither used nor implemented yet.</i>
 *
 * @author Norman Fomferra
 */
public class PyObject {
    public static final long NULL_POINTER = 0;

    private final long pointer;

    PyObject(long pointer) {
        this.pointer = pointer;
    }

    @Override
    protected void finalize() throws Throwable {
        super.finalize();
        PyLib.decref(getPointer());
    }

    public final long getPointer() {
        return pointer;
    }

    public PyObject getAttributeValue(String name) {
        assertLibInitialized();
        long value = PyLib.getAttributeValue(getPointer(), name);
        if (value == NULL_POINTER) {
            throw new RuntimeException("NULL_POINTER");
        }
        return new PyObject(value);
    }

    public void setAttributeValue(String name, PyObject value) {
        assertLibInitialized();
        PyLib.setAttributeValue(getPointer(), name, value.getPointer());
    }

    public Object callMethod(String name, Object... args) {
        assertLibInitialized();
        return PyLib.call(getPointer(), true, name, args);
    }

    public Object call(String name, Object... args) {
        assertLibInitialized();
        return PyLib.call(getPointer(), false, name, args);
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
