/*
 * Copyright 2015 Brockmann Consult GmbH
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package org.jpy;


/**
 * Represents a Python module.
 *
 * @author Norman Fomferra
 * @since 0.7
 */
@SuppressWarnings({"WeakerAccess", "unused"})
public class PyModule extends PyObject {

    /**
     * The Python module's name.
     */
    private final String name;

    PyModule(PyLib lib, String name, long pointer) {
        super(lib, pointer);
        this.name = name;
    }

    /**
     * @return The Python module's name.
     */
    public String getName() {
        return name;
    }

    /**
     * Create a Java proxy instance of this Python module which contains compatible functions to the ones provided in the
     * interface given by the {@code type} parameter.
     *
     * @param type The interface's type.
     * @param <T>  The interface name.
     * @return A (proxy) instance implementing the given interface.
     */
    public <T> T createProxy(Class<T> type) {
        lib.assertPythonRuns();
        //noinspection unchecked
        return (T) createProxy(PyLib.CallableKind.FUNCTION, type);
    }
}
