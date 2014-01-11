package org.jpy.fixtures;

import java.lang.reflect.Array;

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
                Class<?> argClass = arg.getClass();
                argString.append(argClass.getSimpleName());
                argString.append('(');
                if (argClass.isArray()) {
                    stringifyArray(arg, argString);
                } else {
                    stringifyObject(arg, argString);
                }
                argString.append(')');
            } else {
                argString.append("null");
            }
        }
        return argString.toString();
    }

    private static void stringifyObject(Object arg, StringBuilder argString) {
        argString.append(String.valueOf(arg));
    }

    private static void stringifyArray(Object arg, StringBuilder argString) {
        boolean primitive = arg.getClass().getComponentType().isPrimitive();
        int length = Array.getLength(arg);
        for (int i = 0; i < length; i++) {
            Object item = Array.get(arg, i);
            if (i > 0) {
                argString.append(",");
            }
            if (primitive) {
                argString.append(String.valueOf(item));
            } else {
                argString.append(stringifyArgs(item));
            }
        }
    }


}
