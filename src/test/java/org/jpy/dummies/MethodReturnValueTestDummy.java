package org.jpy.dummies;

/**
 * Used as dummy to test various return value types and variants.
 * Note: Please make sure to not add any method overloads to this class.
 * This is done in {@link MethodOverloadTestDummy}.
 *
 * @author Norman Fomferra
 */
@SuppressWarnings("UnusedDeclaration")
public class MethodReturnValueTestDummy {

    public void getVoid() {
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

    public String getString(String string) {
        return string;
    }

    public Thing getObject(Thing object) {
        return object;
    }

    ///////////////////////////////////////////////////////////////////////////////////
    // 1D-Array Return Values

    public boolean[] getArray1D_boolean(boolean item0, boolean item1, boolean item2) {
        return new boolean[]{item0, item1, item2};
    }

    public byte[] getArray1D_byte(byte item0, byte item1, byte item2) {
        return new byte[]{item0, item1, item2};
    }

    public short[] getArray1D_short(short item0, short item1, short item2) {
        return new short[]{item0, item1, item2};
    }

    public int[] getArray1D_int(int item0, int item1, int item2) {
        return new int[]{item0, item1, item2};
    }

    public long[] getArray1D_long(long item0, long item1, long item2) {
        return new long[]{item0, item1, item2};
    }

    public float[] getArray1D_float(float item0, float item1, float item2) {
        return new float[]{item0, item1, item2};
    }

    public double[] getArray1D_double(double item0, double item1, double item2) {
        return new double[]{item0, item1, item2};
    }

    // add other variants

    ///////////////////////////////////////////////////////////////////////////////////
    // 2D-Array Return Values

    public boolean[][] getArray2D_boolean(boolean item00, boolean item01, boolean item10, boolean item11) {
        return new boolean[][]{{item00, item01}, {item10, item11}};
    }

    public byte[][] getArray2D_byte(byte item00, byte item01, byte item10, byte item11) {
        return new byte[][]{{item00, item01}, {item10, item11}};
    }

    public int[][] getArray2D_byte(int item00, int item01, int item10, int item11) {
        return new int[][]{{item00, item01}, {item10, item11}};
    }

    // add other variants
}
