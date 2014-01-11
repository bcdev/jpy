package org.jpy.fixtures;

/**
 * Used as a test class for the test cases in jpy_typeconv_test.py
 *
 * @author Norman Fomferra
 */
@SuppressWarnings("UnusedDeclaration")
public class TypeConversionTestFixture {

    public String stringifyObjectArg(Object arg) {
        return MethodOverloadTestFixture.stringifyArgs(arg);
    }

    public String stringifyIntArrayArg(int[] arg) {
        return MethodOverloadTestFixture.stringifyArgs((Object) arg);
    }
}
