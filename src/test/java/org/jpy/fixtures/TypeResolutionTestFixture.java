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
 * Used as a test class for the test cases in jpy_typeres_test.py
 *
 * @author Norman Fomferra
 */
@SuppressWarnings("UnusedDeclaration")
public class TypeResolutionTestFixture {

    public SuperThing createSuperThing(int value) {
        return new SuperThing(value);
    }


    public static class SuperThing extends Thing {
        public SuperThing(int value) {
            super(value);
        }

        public void add(int val) {
            setValue(getValue() + val);
        }
    }
}
