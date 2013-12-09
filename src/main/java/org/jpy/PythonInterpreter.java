package org.jpy;

/**
 * Represents the CPython interpreter.
 *
 * @author Norman Fomferra
 */
public class PythonInterpreter {
    static {
        System.loadLibrary("jpy");
    }

    public native PyObject getObject(String name);
}
