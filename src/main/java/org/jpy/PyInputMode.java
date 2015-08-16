package org.jpy;

import java.util.Map;

/**
 * Source code input modes for compiling/executing Python source code.
 *
 * @author Norman Fomferra
 * @since 0.8
 * @see PyObject#executeCode(String, PyInputMode)
 * @see PyObject#executeCode(String, PyInputMode, Map, Map)
 */
public enum PyInputMode {
    /**
     * Compile/execute single statement code.
     * <p>
     * The start symbol from the Python grammar for a
     * single statement ({@code Py_single_input} in Python PyRun API).
     * This is the symbol used for the interactive interpreter loop.
     */
    STATEMENT(256),
    /**
     * Compile/execute multi-statement script code.
     * <p>
     * The start symbol from the Python grammar for sequences of
     * statements as read from a file or other source ({@code Py_file_input} in Python PyRun API).
     * This is the symbol to use when compiling arbitrarily long Python source code.
     */
    SCRIPT(257),
    /**
     * Compile/execute single expression.
     * <p>
     * The start symbol from the Python grammar for sequences of
     * statements as read from a file or other source ({@code Py_eval_input} in Python PyRun API).
     */
    EXPRESSION(258);

    public int value() {
        return value;
    }

    private final int value;

    PyInputMode(int value) {
        this.value = value;
    }
}
