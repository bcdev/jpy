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

import static org.jpy.fixtures.MethodOverloadTestFixture.stringifyArgs;

/**
 * Used as a test class for the test cases in jpy_typeconv_test.py
 *
 * @author Norman Fomferra
 */
@SuppressWarnings("UnusedDeclaration")
public class TypeConversionTestFixture {

    public String stringifyObjectArg(Object arg) {
        return stringifyArgs(arg);
    }

    public String stringifyIntArrayArg(int[] arg) {
        return stringifyArgs((Object) arg);
    }

    public String stringifyObjectArrayArg(Object[] arg) {
        return stringifyArgs((Object) arg);
    }

    public String stringifyStringArrayArg(String[] arg) {
        return stringifyArgs((Object) arg);
    }
}
