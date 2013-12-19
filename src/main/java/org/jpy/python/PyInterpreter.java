package org.jpy.python;

import static org.jpy.python.PyLib.assertInterpreterInitialized;

/**
 * Represents the CPython interpreter.
 *
 * @author Norman Fomferra
 */
public class PyInterpreter {

    public static void initialize(String[] options, boolean debug) {
        if (PyLib.isInterpreterInitialized()) {
            throw new RuntimeException();
        }
        PyLib.initializeInterpreter(options, debug);
    }

    public static void destroy() {
        PyLib.destroyInterpreter();
    }

    public PyModule importModule(String name) {
        assertInterpreterInitialized();
        long modulePointer = PyLib.importModule(name);
        return new PyModule(name, modulePointer);
    }

    public PyObject getObject(PyModule module, String name) {
        assertInterpreterInitialized();
        long objectPointer = PyLib.getAttributeObject(module.getPointer(), name);
        return new PyObject(objectPointer);
    }

    public void execScript(String script) {
        assertInterpreterInitialized();
        PyLib.execScript(script);
    }
}
