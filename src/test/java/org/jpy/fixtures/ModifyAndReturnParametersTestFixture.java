package org.jpy.fixtures;

import org.jpy.annotations.Mutable;
import org.jpy.annotations.Return;

/**
 * Used as a test class for the test cases in jpy_modretparam_test.py
 *
 * @author Norman Fomferra
 */
@SuppressWarnings("UnusedDeclaration")
public class ModifyAndReturnParametersTestFixture {

    public void modifyThing(@Mutable Thing thing, int value) {
        thing.setValue(value);
    }

    public Thing returnThing(@Return Thing thing) {
        if (thing == null) {
            thing = new Thing();
        }
        return thing;
    }

    public Thing modifyAndReturnThing(@Mutable @Return Thing thing, int value) {
        if (thing == null) {
            thing = new Thing();
        }
        thing.setValue(value);
        return thing;
    }

    public void modifyIntArray(@Mutable int[] array, int item0, int item1, int item2) {
        array[0] = item0;
        array[1] = item1;
        array[2] = item2;
    }

    public int[] returnIntArray(@Return int[] array) {
        if (array == null) {
            array = new int[3];
        }
        return array;
    }

    public int[] modifyAndReturnIntArray(@Mutable @Return int[] array, int item0, int item1, int item2) {
        if (array == null) {
            array = new int[3];
        }
        array[0] = item0;
        array[1] = item1;
        array[2] = item2;
        return array;
    }
}
