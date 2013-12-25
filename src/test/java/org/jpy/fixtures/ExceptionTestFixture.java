package org.jpy.fixtures;

import java.io.IOException;

/**
 * Used as a test class for the test cases in jpy_exception_test.py
 *
 * @author Norman Fomferra
 */
public class ExceptionTestFixture {
    public int throwNpeIfArgIsNull(String arg) {
        return arg.length();
    }

    public int throwAioobeIfIndexIsNotZero(int index) {
        int[] ints = new int[]{101};
        return ints[index];
    }


    public void throwRteIfMessageIsNotNull(String message) {
        if (message != null) {
            throw new RuntimeException(message);
        }
    }

    public void throwIoeIfMessageIsNotNull(String message) throws IOException {
        if (message != null) {
            throw new IOException(message);
        }
    }

}
