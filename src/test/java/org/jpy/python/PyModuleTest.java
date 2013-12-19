package org.jpy.python;

import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Test;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertTrue;

/**
 * @author Norman Fomferra
 */
public class PyModuleTest {
    @BeforeClass
    public static void setUp() throws Exception {
        assertEquals(false, PyLib.isInterpreterInitialized());
        PyLib.initializeInterpreter(new String[0], false);
        assertEquals(true, PyLib.isInterpreterInitialized());
    }

    @AfterClass
    public static void tearDown() throws Exception {
        PyLib.destroyInterpreter();
    }

    @Test
    public void testCast() throws Exception {
        PyModule procModule = PyModule.importModule("proc_module");
        Proc proc = procModule.cast(Proc.class);
        assertNotNull(proc);
        String result;
        result = proc.initialize();
        assertEquals("initialize-1", result);
        result = proc.computeTile(100, 100, new float[100 * 100]);
        assertEquals("computeTile-2", result);
        result = proc.computeTile(100, 100, new float[100 * 100]);
        assertEquals("computeTile-3", result);
        result = proc.computeTile(100, 100, new float[100 * 100]);
        assertEquals("computeTile-4", result);
        result = proc.dispose();
        assertEquals("dispose-5", result);
    }

}
