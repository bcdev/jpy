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
 * @since 0.7
 */
public class PyModule extends PyObject {

    /**
     * The Python module's name.
     */
    private final String name;

    PyModule(String name, long pointer) {
        super(pointer);
        this.name = name;
    }

    /**
     * @return The Python module's name.
     */
    public String getName() {
        return name;
    }


    /**
     * Import a Python module into the Python interpreter and return its Java representation.
     *
     * @param name The Python module's name.
     * @return The Python module's Java representation.
     */
    public static PyModule importModule(String name) {
        assertPythonRuns();
        long pointer = PyLib.importModule(name);
        return pointer != 0 ? new PyModule(name, pointer) : null;
    }

    /**
     * Create a Java proxy instance of this Python module which contains compatible functions to the ones provided in the
     * interface given by the {@code type} parameter.
     *
     * @param type The interface's type.
     * @param <T> The interface name.
     * @return A (proxy) instance implementing the given interface.
     */
    public <T> T createProxy(Class<T> type) {
        assertPythonRuns();
        return (T) createProxy(PyLib.CallableKind.FUNCTION, type);
    }
}
