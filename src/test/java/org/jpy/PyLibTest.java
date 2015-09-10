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

import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Test;

import static org.junit.Assert.*;

public class PyLibTest {

    @BeforeClass
    public static void setUpClass() throws Exception {
        //PyLib.Diag.setFlags(PyLib.Diag.F_ERR);
        PyLib.startPython();
        assertEquals(true, PyLib.isPythonRunning());
    }

    @AfterClass
    public static void tearDownClass() throws Exception {
        //PyLib.stopPython();
    }

    @Test
    public void testGetPythonVersion() throws Exception {
        String pythonVersion = PyLib.getPythonVersion();
        System.out.println("pythonVersion = " + pythonVersion);
        assertNotNull(pythonVersion);
    }

    @Test
    public void testExecScript() throws Exception {
        int exitCode = PyLib.execScript(String.format("print('%s says: \"Hello Python!\"')", PyLibTest.class.getName()));
        assertEquals(0, exitCode);
    }

    @Test
    public void testExecScriptInError() throws Exception {
        int exitCode;
        exitCode = PyLib.execScript("0 / 1");
        assertEquals(0, exitCode);
        exitCode = PyLib.execScript("1 / 0");
        assertEquals(-1, exitCode);
    }

    @Test
    public void testImportModule() throws Exception {
        long pyModule;

        pyModule = PyLib.importModule("os");
        assertTrue(pyModule != 0);

        pyModule = PyLib.importModule("sys");
        assertTrue(pyModule != 0);
    }

    @Test
    public void testGetSetAttributeValue() throws Exception {
        long pyModule;
        long pyObject;

        pyModule = PyLib.importModule("jpy");
        assertTrue(pyModule != 0);

        long pyObj = PyLib.getAttributeObject(pyModule, "JType");
        assertTrue(pyObj != 0);

        PyLib.setAttributeValue(pyModule, "_hello", "Hello Python!", String.class);
        assertEquals("Hello Python!", PyLib.getAttributeValue(pyModule, "_hello", String.class));

        pyObject = PyLib.getAttributeObject(pyModule, "_hello");
        assertTrue(pyObject != 0);
    }

    @Test
    public void testCallAndReturnValue() throws Exception {
        long builtins;

        try {
            //Python 3.3
            builtins = PyLib.importModule("builtins");
        } catch (Exception e) {
            //Python 2.7
            builtins = PyLib.importModule("__builtin__");
        }
        assertTrue(builtins != 0);

        long max = PyLib.getAttributeObject(builtins, "max");
        assertTrue(max != 0);

        //PyLib.Diag.setFlags(PyLib.Diag.F_ALL);
        String result = PyLib.callAndReturnValue(builtins, false, "max", 2, new Object[]{"A", "Z"}, new Class[]{String.class, String.class}, String.class);

        assertEquals("Z", result);
    }

    @Test
    public void testCallAndReturnObject() throws Exception {
        long builtins;
        long pointer;

        try {
            //Python 3.3
            builtins = PyLib.importModule("builtins");
        } catch (Exception e) {
            //Python 2.7
            builtins = PyLib.importModule("__builtin__");
        }
        assertTrue(builtins != 0);

        long max = PyLib.getAttributeObject(builtins, "max");
        assertTrue(max != 0);

        pointer = PyLib.callAndReturnObject(builtins, false, "max", 2, new Object[]{"A", "Z"}, null);
        assertTrue(pointer != 0);

        //PyLib.Diag.setFlags(PyLib.Diag.F_ALL);
        assertEquals("Z", new PyObject(pointer).getStringValue());
    }
}
