package org.jpy;

/**
 * Represents a CPython object (of type <code>PyObject *</code>).
 * @author Norman Fomferra
 */
public interface PyObject {
    long getPointer();
}
