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

import java.util.Collection;
import java.util.Collections;
import org.junit.*;

import static org.junit.Assert.*;

import java.util.Map;

public class PyLibTest {

    @Before
    public void setUp() throws Exception {
        //PyLib.Diag.setFlags(PyLib.Diag.F_ERR);
        PyLib.startPython();
        assertEquals(true, PyLib.isPythonRunning());
    }

    @After
    public void tearDown() throws Exception {
        PyLib.stopPython();
    }

    @Test
    public void testGettingSysArgv() throws Exception {
        // Since sys.argv is really part of the C-based sys module, there
        // are special hooks embedded python systems need to call to set it up.
        PyModule sys = PyModule.importModule("sys");
        String[] argv = sys.getAttribute("argv", String[].class);
        assertNotNull(argv);
        assertEquals(1, argv.length);
        assertTrue(argv[0].isEmpty());
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

    @Test
    public void testGetMainGlobals() throws Exception {
        PyObject globals = PyLib.getMainGlobals();
        Map<PyObject, PyObject> dict = globals.asDict();
        assertFalse(dict.isEmpty());

        boolean foundName = false;

        for (Map.Entry<PyObject, PyObject> entry : dict.entrySet()) {
            if (entry.getKey().isString()) {
                if (entry.getKey().getObjectValue().equals("__name__")) {
                    foundName = true;
                    break;
                }
            }
        }

        assertTrue(foundName);
    }

    @Test
    public void testNewDict() throws Exception {
        PyObject dict = PyLib.newDict();
        assertTrue(dict.asDict().isEmpty());

        PyObject globals = PyLib.getMainGlobals();

        PyObject.executeCode("x = 42", PyInputMode.STATEMENT, globals, dict);

        assertFalse(dict.asDict().isEmpty());
    }

    @Test
    public void testDictKeys() {
        PyObject dict = PyLib.newDict();
        PyDictWrapper wrapper = dict.asDict();
        assertTrue(wrapper.keySet().isEmpty());

        PyObject.executeCode("my_key = 42", PyInputMode.STATEMENT, PyLib.getMainGlobals(), dict);
        PyObject pyKey = PyObject.executeCode("'my_key'", PyInputMode.EXPRESSION);

        assertEquals(wrapper.keySet(), Collections.singleton(pyKey));
        assertTrue(wrapper.containsKey(pyKey));
        assertTrue(wrapper.containsKey("my_key"));
        assertTrue(wrapper.containsKey((Object)pyKey));

        assertFalse(wrapper.containsKey(1));
        assertFalse(wrapper.containsKey(this));
    }

    @Test
    public void testDictValues() {
        PyObject dict = PyLib.newDict();
        PyDictWrapper wrapper = dict.asDict();
        assertTrue(wrapper.values().isEmpty());

        PyObject.executeCode("my_key = 42", PyInputMode.STATEMENT, PyLib.getMainGlobals(), dict);
        PyObject pyValue = PyObject.executeCode("42", PyInputMode.EXPRESSION);

        // PyListWrapper doesn't implement the full List interface... can't check direct List equality
        Collection<PyObject> values = wrapper.values();
        assertFalse(values.isEmpty());
        assertEquals(values.size(), 1);
        assertEquals(values.iterator().next(), pyValue);
        assertTrue(wrapper.containsValue(pyValue));
    }

    @Test
    public void invalidPyDictKeys() {
        PyObject pyValue = PyObject.executeCode("42", PyInputMode.EXPRESSION);
        try {
            PyLib.pyDictKeys(pyValue.getPointer());
            fail("Expected exception");
        } catch (UnsupportedOperationException e) {
            // expected
        }
    }

    @Test
    public void invalidPyDictValues() {
        PyObject pyValue = PyObject.executeCode("42", PyInputMode.EXPRESSION);
        try {
            PyLib.pyDictValues(pyValue.getPointer());
            fail("Expected exception");
        } catch (UnsupportedOperationException e) {
            // expected
        }
    }

    @Test
    public void invalidPyDictContains() {
        PyObject pyValue = PyObject.executeCode("42", PyInputMode.EXPRESSION);
        try {
            PyLib.pyDictContains(pyValue.getPointer(), pyValue, null);
            fail("Expected exception");
        } catch (UnsupportedOperationException e) {
            // expected
        }
    }
}
