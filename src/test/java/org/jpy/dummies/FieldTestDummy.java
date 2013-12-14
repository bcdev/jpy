package org.jpy.dummies;

/**
 * Used as a test class for the test cases in jpy_dummies_test.py
 *
 * @author Norman Fomferra
 */
@SuppressWarnings("UnusedDeclaration")
public class FieldTestDummy {

    public static final boolean z_STATIC_FIELD = true;
    public static final byte b_STATIC_FIELD = (byte) 123;
    public static final char c_STATIC_FIELD = 'A';
    public static final short s_STATIC_FIELD = (short) 12345;
    public static final int i_STATIC_FIELD = 123456789;
    public static final long j_STATIC_FIELD = 1234567890123456789L;
    public static final float f_STATIC_FIELD = 0.12345F;
    public static final double d_STATIC_FIELD = 0.123456789;
    public static final Object S_STATIC_FIELD = "ABC";
    public static final Object l_STATIC_FIELD = new Thing();

    public boolean zInstField;
    public byte bInstField;
    public char cInstField;
    public short sInstField;
    public int iInstField;
    public long jInstField;
    public float fInstField;
    public double dInstField;
    public String SInstField;
    public Object lInstField;
}
