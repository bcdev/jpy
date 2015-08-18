package org.jpy.fixtures;

/**
 * Used as a test class for the test cases in jpy_overload_test.py
 *
 * @author Norman Fomferra
 */
public class ObjWithObjOverrides implements IfcWithObjOverrides {

    @Override
    public String toString() {
        return "Hi!";
    }
}
