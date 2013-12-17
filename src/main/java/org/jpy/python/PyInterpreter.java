package org.jpy.python;

import static org.jpy.python.PyLib.assertLibInitialized;

/**
 * Represents the CPython interpreter.
 *
 * @author Norman Fomferra
 */
public class PyInterpreter {

    public static void initialize(String[] options, boolean debug) {
        if (PyLib.isInitialized()) {
            throw new RuntimeException();
        }
        PyLib.initialize(options, debug);
    }

    public static void destroy() {
        PyLib.destroy();
    }

    public PyModule importModule(String name) {
        assertLibInitialized();
        long modulePointer = PyLib.importModule(name);
        return new PyModule(name, modulePointer);
    }

    public PyObject getObject(PyModule module, String name) {
        assertLibInitialized();
        long objectPointer = PyLib.getAttributeValue(module.getPointer(), name);
        return new PyObject(objectPointer);
    }

    public void execScript(String script) {
        assertLibInitialized();
        PyLib.execScript(script);
    }
}
