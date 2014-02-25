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
 * @author Norman Fomferra
 */
@SuppressWarnings("UnusedDeclaration")
public class ConstructorOverloadTestFixture {
    String state;

    public ConstructorOverloadTestFixture() {
        initState();
    }

    public ConstructorOverloadTestFixture(int a) {
        initState(a);
    }

    public ConstructorOverloadTestFixture(int a, int b) {
        initState(a, b);
    }

    public ConstructorOverloadTestFixture(float a) {
        initState(a);
    }

    public ConstructorOverloadTestFixture(float a, float b) {
        initState(a, b);
    }

    public ConstructorOverloadTestFixture(int a, float b) {
        initState(a, b);
    }

    public ConstructorOverloadTestFixture(float a, int b) {
        initState(a, b);
    }

    public String getState() {
        return state;
    }

    private void initState(Object... args) {
        state = MethodOverloadTestFixture.stringifyArgs(args);
    }
}
