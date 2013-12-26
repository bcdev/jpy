package org.jpy.fixtures;

import org.jpy.annotations.Mutable;
import org.jpy.annotations.Return;

/**
 * todo - write a Python test case for me!
 *
 * @author Norman Fomferra
 */
@SuppressWarnings("UnusedDeclaration")
public class MethodArgumentTestFixture {

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
