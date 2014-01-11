package org.jpy.fixtures;

/**
 * Used as a test class for the test cases in jpy_typeres_test.py
 *
 * @author Norman Fomferra
 */
@SuppressWarnings("UnusedDeclaration")
public class TypeResolutionTestFixture {

    public Thing createThing(int value) {
        return new Thing(value);
    }
}
