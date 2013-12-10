package org.jpy.dummies;

import org.jpy.Mutable;
import org.jpy.Return;

/**
 * Used as a test class for the test cases in jpy_dummies_test.py
 *
 * @author Norman Fomferra
 */
@SuppressWarnings("UnusedDeclaration")
public class MethodArgumentTestDummy {

    public void modifyObject(@Mutable Thing object, int value) {
        object.setValue(value);
    }

    public int[] getArray1D_int(@Return int[] array) {
        return array;
    }

    public int[] modifyArray1D_int(@Mutable @Return int[] value, int item0, int item1, int item2) {
        value[0] = item0;
        value[1] = item1;
        value[2] = item2;
        return value;
    }
}
