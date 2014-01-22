package org.jpy;

import static org.jpy.PyLib.assertInterpreterInitialized;

/**
 * Represents a Python module.
 *
 * @author Norman Fomferra
 * @since 1.0
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

    public <T> T createProxy(Class<T> type) {
        assertInterpreterInitialized();
        return (T) createProxy(PyLib.CallableKind.FUNCTION, type);
    }
}
