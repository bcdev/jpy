package org.jpy.python;

import static org.jpy.python.PyLib.assertInterpreterInitialized;

/**
 * Represents the CPython interpreter.
 *
 * @author Norman Fomferra
 */
public class PyInterpreter {

    public static void initialize(String[] options) {
        if (PyLib.isInterpreterInitialized()) {
            throw new RuntimeException();
        }
        PyLib.initializeInterpreter(options);
    }

    public static void destroy() {
        PyLib.destroyInterpreter();
    }

    public static PyModule importModule(String name) {
        assertInterpreterInitialized();
        long modulePointer = PyLib.importModule(name);
        return new PyModule(name, modulePointer);
    }

    public static PyObject getObject(PyModule module, String name) {
        assertInterpreterInitialized();
        long objectPointer = PyLib.getAttributeObject(module.getPointer(), name);
        return new PyObject(objectPointer);
    }

    public static void execScript(String script) {
        assertInterpreterInitialized();
        PyLib.execScript(script);
    }
}
