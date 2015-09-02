package org.jpy.fixtures;

/**
 * Used as a test class for the test cases in jpy_overload_test.py
 *
 * @author Norman Fomferra
 */
public interface IfcWithObjOverrides {
    /**
     * Force overriding java.lang.Object method.
     */
    @Override
    String toString();

    /**
     * In opposite to toString(), this is a new method.
     */
    String getName();
}
