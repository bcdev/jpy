package org.jpy;

/**
 * Represents the CPython interpreter.
 *
 * <p/>
 * <i>Neither used nor implemented yet.</i>
 *
 * @author Norman Fomferra
 */
public class PythonInterpreter {
    static {
        System.loadLibrary("jpy");
    }

    public native PyObject getObject(String name);
}
