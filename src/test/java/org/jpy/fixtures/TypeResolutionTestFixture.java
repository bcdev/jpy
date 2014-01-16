package org.jpy.fixtures;

/**
 * Used as a test class for the test cases in jpy_typeres_test.py
 *
 * @author Norman Fomferra
 */
@SuppressWarnings("UnusedDeclaration")
public class TypeResolutionTestFixture {

    public SuperThing createSuperThing(int value) {
        return new SuperThing(value);
    }


    public static class SuperThing extends Thing {
        public SuperThing(int value) {
            super(value);
        }

        public void add(int val) {
            setValue(getValue() + val);
        }
    }
}
