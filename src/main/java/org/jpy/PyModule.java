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

package org.jpy;

import static org.jpy.PyLib.assertPythonRuns;

/**
 * Represents a Python module.
 *
 * @author Norman Fomferra
 * @since 1.0
 */
public class PyModule extends PyObject {
    private final String name;

    PyModule(String name, long pointer) {
        super(pointer);
        this.name = name;
    }

    public String getName() {
        return name;
    }

    public static PyModule importModule(String name) {
        assertPythonRuns();
        long pointer = PyLib.importModule(name);
        return pointer != 0 ? new PyModule(name, pointer) : null;
    }

    public <T> T createProxy(Class<T> type) {
        assertPythonRuns();
        return (T) createProxy(PyLib.CallableKind.FUNCTION, type);
    }
}
