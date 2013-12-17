package org.jpy.python;

/**
 * Represents a CPython object (of type <code>PyObject *</code>).
 *
 * @author Norman Fomferra
 */
public class PyModule extends PyObject {
    private final String name;

    PyModule(String name, long pointer) {
        super(pointer);
        this.name = name;
    }

    public String getName() {
        return name;
    }

    public static PyModule importModule(String name) {
        PyLib.assertInterpreterInitialized();
        long pointer = PyLib.importModule(name);
        return new PyModule(name, pointer);
    }

    @Override
    protected PyInvocationHandler createInvocationHandler() {
        return new PyInvocationHandler(this, false);
    }
}
