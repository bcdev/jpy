package org.jpy.fixtures;

import static org.jpy.fixtures.MethodOverloadTestFixture.stringifyArgs;

/**
 * Used as a test class for the test cases in jpy_typeconv_test.py
 *
 * @author Norman Fomferra
 */
@SuppressWarnings("UnusedDeclaration")
public class TypeConversionTestFixture {

    public String stringifyObjectArg(Object arg) {
        return stringifyArgs(arg);
    }

    public String stringifyIntArrayArg(int[] arg) {
        return stringifyArgs((Object) arg);
    }

    public String stringifyObjectArrayArg(Object[] arg) {
        return stringifyArgs((Object) arg);
    }

    public String stringifyStringArrayArg(String[] arg) {
        return stringifyArgs((Object) arg);
    }
}
