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
