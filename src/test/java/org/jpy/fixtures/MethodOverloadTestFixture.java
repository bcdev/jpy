package org.jpy.fixtures;

/**
 * Used as a test class for the test cases in jpy_overload_test.py
 *
 * @author Norman Fomferra
 */
@SuppressWarnings("UnusedDeclaration")
public class MethodOverloadTestFixture {

    public String join(int a, int b) {
        return "I" + a + " I" + b;
    }

    public String join(int a, double b) {
        return "I" + a + " D" + b;
    }

    public String join(int a, String b) {
        return "I" + a + " S" + b;
    }

    public String join(double a, int b) {
        return "D" + a + " I" + b;
    }

    public String join(double a, double b) {
        return "D" + a + " D" + b;
    }

    public String join(double a, String b) {
        return "D" + a + " S" + b;
    }

    public String join(String a, int b) {
        return "S" + a + " I" + b;
    }

    public String join(String a, double b) {
        return "S" + a + " D" + b;
    }

    public String join(String a, String b) {
        return "S" + a + " S" + b;
    }

    //////////////////////////////////////////////

    public String join(String a) {
        return "S" + a;
    }

    public String join(String a, String b, String c) {
        return "S" + a + " S" + b + " S" + c;
    }

    /**
     * Used to test that we also find overloaded methods in class hierarchies
     */
    public static class MethodOverloadTestFixture2 extends MethodOverloadTestFixture {

        public String join(String a, String b, String c, String d) {
            return "S" + a + " S" + b + " S" + c + " S" + d;
        }
    }

    //////////////////////////////////////////////

    // Should never been found, since 'float' is not present in Python
    public String join(int a, float b) {
        return "I" + a + " F" + b;
    }

}
