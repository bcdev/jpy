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

import org.junit.*;

import static org.junit.Assert.*;

public class PyLibTest extends PyLibTestBase {

    @Before
    public void setUp() throws Exception {
        //lib.Diag.setDiagFlags(lib.Diag.F_ERR);
        lib.startPython();
        assertEquals(true, lib.isPythonRunning());
    }

    @Test
    public void testGettingSysArgv() throws Exception {
        // Since sys.argv is really part of the C-based sys module, there
        // are special hooks embedded python systems need to call to set it up.
        PyModule sys = lib.importModule("sys");
        String[] argv = sys.getAttribute("argv", String[].class);
        assertNotNull(argv);
        assertEquals(1, argv.length);
        assertTrue(argv[0].isEmpty());
    }

    @Test
    public void testGetPythonVersion() throws Exception {
        String pythonVersion = lib.getPythonVersion();
        System.out.println("pythonVersion = " + pythonVersion);
        assertNotNull(pythonVersion);
    }

    @Test
    public void testExecScript() throws Exception {
        int exitCode = lib.execScript(String.format("print('%s says: \"Hello Python!\"')", PyLibTest.class.getName()));
        assertEquals(0, exitCode);
    }

    @Test
    public void testExecScriptInError() throws Exception {
        int exitCode;
        exitCode = lib.execScript("0 / 1");
        assertEquals(0, exitCode);
        exitCode = lib.execScript("1 / 0");
        assertEquals(-1, exitCode);
    }

    @Test
    public void testImportModule() throws Exception {
        long pyModule;

        pyModule = lib.importModule0("os");
        assertTrue(pyModule != 0);

        pyModule = lib.importModule0("sys");
        assertTrue(pyModule != 0);
    }

    @Test
    public void testGetSetAttributeValue() throws Exception {
        long pyModule;
        long pyObject;

        pyModule = lib.importModule0("jpy");
        assertTrue(pyModule != 0);

        long pyObj = lib.getAttributeObject(pyModule, "JType");
        assertTrue(pyObj != 0);

        lib.setAttributeValue(pyModule, "_hello", "Hello Python!", String.class);
        assertEquals("Hello Python!", lib.getAttributeValue(pyModule, "_hello", String.class));

        pyObject = lib.getAttributeObject(pyModule, "_hello");
        assertTrue(pyObject != 0);
    }

    @Test
    public void testCallAndReturnValue() throws Exception {
        long builtins;

        try {
            //Python 3.3
            builtins = lib.importModule0("builtins");
        } catch (Exception e) {
            //Python 2.7
            builtins = lib.importModule0("__builtin__");
        }
        assertTrue(builtins != 0);

        long max = lib.getAttributeObject(builtins, "max");
        assertTrue(max != 0);

        //lib.Diag.setDiagFlags(lib.Diag.F_ALL);
        String result = lib.callAndReturnValue(builtins, false, "max", 2, new Object[]{"A", "Z"}, new Class[]{String.class, String.class}, String.class);

        assertEquals("Z", result);
    }

    @Test
    public void testCallAndReturnObject() throws Exception {
        long builtins;
        long pointer;

        try {
            //Python 3.3
            builtins = lib.importModule0("builtins");
        } catch (Exception e) {
            //Python 2.7
            builtins = lib.importModule0("__builtin__");
        }
        assertTrue(builtins != 0);

        long max = lib.getAttributeObject(builtins, "max");
        assertTrue(max != 0);

        pointer = lib.callAndReturnObject(builtins, false, "max", 2, new Object[]{"A", "Z"}, null);
        assertTrue(pointer != 0);

        //lib.Diag.setDiagFlags(lib.Diag.F_ALL);
        assertEquals("Z", new PyObject(lib, pointer).getStringValue());
    }
}
