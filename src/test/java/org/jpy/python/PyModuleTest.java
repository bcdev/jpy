package org.jpy.python;

import org.jpy.fixtures.Processor;
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
        Processor processor = procModule.cast(Processor.class);
        assertNotNull(processor);
        String result;
        result = processor.initialize();
        assertEquals("initialize-1", result);
        result = processor.computeTile(100, 100, new float[100 * 100]);
        assertEquals("computeTile-2", result);
        result = processor.computeTile(100, 100, new float[100 * 100]);
        assertEquals("computeTile-3", result);
        result = processor.computeTile(100, 100, new float[100 * 100]);
        assertEquals("computeTile-4", result);
        result = processor.dispose();
        assertEquals("dispose-5", result);
    }

}
