package org.jpy.python;

import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Ignore;
import org.junit.Test;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;

public class PyLibTest {
    @BeforeClass
    public static void setUp() throws Exception {
        assertEquals(false, PyLib.isInterpreterInitialized());
        PyLib.initializeInterpreter(new String[0], false);
        assertEquals(true, PyLib.isInterpreterInitialized());
    }

    @AfterClass
    public static void tearDown() throws Exception {
        assertEquals(true, PyLib.isInterpreterInitialized());
        PyLib.destroyInterpreter();
        assertEquals(false, PyLib.isInterpreterInitialized());
    }

    @Test
    public void testExecScript() throws Exception {
        // Just call, no special test
        PyLib.execScript("print('Hello Python!')");
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

        long pyObj = PyLib.getAttributeValue(pyModule, "JType");
        assertTrue(pyObj != 0);

        PyLib.setAttributeValue(pyModule, "_hello", "Hello Python!", String.class);

        pyObject = PyLib.getAttributeValue(pyModule, "_hello");
        assertTrue(pyObject != 0);
    }

    @Test
    public void testCallWithParamTypes() throws Exception {
        long builtins;
        long result;

        builtins = PyLib.importModule("builtins");
        assertTrue(builtins != 0);

        result = PyLib.call(builtins, false, "min", 2,
                            new Object[]{"A", "Z"},
                            new Class[] {String.class, String.class});

        assertTrue(result != 0);
    }

    @Test
    public void testCallWithoutParamTypes() throws Exception {
        long builtins;
        long result;

        builtins = PyLib.importModule("builtins");
        assertTrue(builtins != 0);

        result = PyLib.call(builtins, false, "min", 2,
                            new Object[]{"A", "Z"},
                            null);

        assertTrue(result != 0);
    }
}
