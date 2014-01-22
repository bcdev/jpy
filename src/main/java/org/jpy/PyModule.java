package org.jpy;

import static org.jpy.PyLib.assertPythonRuns;

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
        assertPythonRuns();
        long pointer = PyLib.importModule(name);
        return pointer != 0 ? new PyModule(name, pointer) : null;
    }

    public <T> T createProxy(Class<T> type) {
        assertPythonRuns();
        return (T) createProxy(PyLib.CallableKind.FUNCTION, type);
    }
}
