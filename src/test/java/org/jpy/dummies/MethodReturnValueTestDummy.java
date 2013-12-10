package org.jpy.dummies;

import org.jpy.ReturnValue;

/**
 * Used as dummy to test various return value types and variants.
 * Note: Please make sure to not add any method overloads to this class.
 * This is done in {@link MethodOverloadTestDummy}.
 *
 * @author Norman Fomferra
 */
@SuppressWarnings("UnusedDeclaration")
public class MethodReturnValueTestDummy {

    private final int id;

    public MethodReturnValueTestDummy() {
        this.id = 0;
    }

    public MethodReturnValueTestDummy(int id) {
        this.id = id;
    }

    public int getId() {
        return id;
    }

    public void getValue_void() {
    }

    public boolean getValue_boolean(boolean value) {
        return value;
    }

    public byte getValue_byte(byte value) {
        return value;
    }

    public short getValue_short(short value) {
        return value;
    }

    public int getValue_int(int value) {
        return value;
    }

    public long getValue_long(long value) {
        return value;
    }

    public float getValue_float(float value) {
        return value;
    }

    public double getValue_double(double value) {
        return value;
    }

    public String getValue_String(String value) {
        return value;
    }

    public MethodReturnValueTestDummy getValue_Object(@ReturnValue MethodReturnValueTestDummy value) {
        return value;
    }

    public MethodReturnValueTestDummy createValue_Object(int id) {
        return new MethodReturnValueTestDummy(id);
    }

    ///////////////////////////////////////////////////////////////////////////////////
    // 1D-Array Return Values

    public boolean[] getArray1D_boolean(@ReturnValue boolean[] value) {
        return value;
    }

    public boolean[] getArray1D_boolean(boolean item0, boolean item1, boolean item2) {
        return new boolean[]{item0, item1, item2};
    }

    public byte[] getArray1D_byte(@ReturnValue byte[] value) {
        return value;
    }

    public byte[] getArray1D_byte(byte item0, byte item1, byte item2) {
        return new byte[]{item0, item1, item2};
    }

    // add other variants

    ///////////////////////////////////////////////////////////////////////////////////
    // 2D-Array Return Values

    public boolean[][] getArray2D_boolean(@ReturnValue boolean[][] value) {
        return value;
    }

    public boolean[][] getArray2D_boolean(boolean item00, boolean item01, boolean item10, boolean item11) {
        return new boolean[][]{{item00, item01}, {item10, item11}};
    }

    public byte[][] getArray2D_byte(@ReturnValue byte[][] value) {
        return value;
    }

    public byte[][] getArray2D_byte(byte item00, byte item01, byte item10, byte item11) {
        return new byte[][]{{item00, item01}, {item10, item11}};
    }

    // add other variants
}
