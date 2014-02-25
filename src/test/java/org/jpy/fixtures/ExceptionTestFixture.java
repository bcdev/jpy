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

import java.io.IOException;

/**
 * Used as a test class for the test cases in jpy_exception_test.py
 *
 * @author Norman Fomferra
 */
@SuppressWarnings("UnusedDeclaration")
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
