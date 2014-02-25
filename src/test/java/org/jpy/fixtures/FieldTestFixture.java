/*
 * Copyright (C) 2014 Brockmann Consult GmbH
 *
 * This program is free software; you can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation; either version 3 of the License,
 * or (at your option) any later version. This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
 * PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program; if not, see
 * http://www.gnu.org/licenses/
 */

package org.jpy.fixtures;

/**
 * Used as a test class for the test cases in jpy_field_test.py
 *
 * @author Norman Fomferra
 */
@SuppressWarnings("UnusedDeclaration")
public class FieldTestFixture {

    public static final boolean z_STATIC_FIELD = true;
    public static final char c_STATIC_FIELD = 'A';
    public static final byte b_STATIC_FIELD = (byte) 123;
    public static final short s_STATIC_FIELD = (short) 12345;
    public static final int i_STATIC_FIELD = 123456789;
    public static final long j_STATIC_FIELD = 1234567890123456789L;
    public static final float f_STATIC_FIELD = 0.12345F;
    public static final double d_STATIC_FIELD = 0.123456789;

    public static final String S_OBJ_STATIC_FIELD = "ABC";
    public static final Thing l_OBJ_STATIC_FIELD = new Thing(123);

    public boolean zInstField;
    public char cInstField;
    public byte bInstField;
    public short sInstField;
    public int iInstField;
    public long jInstField;
    public float fInstField;
    public double dInstField;

    public Boolean zObjInstField;
    public Character cObjInstField;
    public Byte bObjInstField;
    public Short sObjInstField;
    public Integer iObjInstField;
    public Long jObjInstField;
    public Float fObjInstField;
    public Double dObjInstField;

    public String SObjInstField;
    public Thing lObjInstField;
}
