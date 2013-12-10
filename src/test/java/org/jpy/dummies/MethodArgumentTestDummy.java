package org.jpy.dummies;

import org.jpy.ReturnValue;

/**
 * Used as a test class for the test cases in jpy_jvm_librt_test.py
 *
 * @author Norman Fomferra
 */
@SuppressWarnings("UnusedDeclaration")
public class MethodArgumentTestDummy {

    private final int id;

    public MethodArgumentTestDummy(int id) {
        this.id = id;
    }

    public int getId() {
        return id;
    }

    // todo: add variants here
}
