package org.jpy.fixtures;

/**
 * Used as a test class for the test cases in jpy_overload_test.py
 *
 * @author Norman Fomferra
 */
@SuppressWarnings("UnusedDeclaration")
public class MethodOverloadTestFixture {

    public String join(int a, int b) {
        return stringifyArgs(a, b);
    }

    public String join(int a, double b) {
        return stringifyArgs(a, b);
    }

    public String join(int a, String b) {
        return stringifyArgs(a, b);
    }

    public String join(double a, int b) {
        return stringifyArgs(a, b);
    }

    public String join(double a, double b) {
        return stringifyArgs(a, b);
    }

    public String join(double a, String b) {
        return stringifyArgs(a, b);
    }

    public String join(String a, int b) {
        return stringifyArgs(a, b);
    }

    public String join(String a, double b) {
        return stringifyArgs(a, b);
    }

    public String join(String a, String b) {
        return stringifyArgs(a, b);
    }

    //////////////////////////////////////////////

    public String join(String a) {
        return stringifyArgs(a);
    }

    public String join(String a, String b, String c) {
        return stringifyArgs(a, b, c);
    }

    /**
     * Used to test that we also find overloaded methods in class hierarchies
     */
    public static class MethodOverloadTestFixture2 extends MethodOverloadTestFixture {

        public String join(String a, String b, String c, String d) {
            return stringifyArgs(a, b, c, d);
        }
    }

    //////////////////////////////////////////////

    // Should never been found, since 'float' is not present in Python
    public String join(int a, float b) {
        return stringifyArgs(a, b);
    }

    static String stringifyArgs(Object... args) {
        StringBuilder argString = new StringBuilder();
        for (int i = 0; i < args.length; i++) {
            if (i > 0) {
                argString.append(",");
            }
            Object arg = args[i];
            if (arg != null) {
                argString.append(arg.getClass().getSimpleName());
                argString.append('(');
                argString.append(String.valueOf(arg));
                argString.append(')');
            } else {
                argString.append("null");
            }
        }
        return argString.toString();
    }


}
