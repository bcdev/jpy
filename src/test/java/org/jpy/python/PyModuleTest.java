package org.jpy.python;

import org.jpy.fixtures.Processor;
import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Test;

import java.io.File;

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
        // Add module dir to sys.path in order to import file 'proc_module.py'
        String importPath = new File("src/test/python/fixtures").getCanonicalPath();
        //System.out.println("importPath = " + importPath);
        PyLib.execScript(String.format("import sys; sys.path.append('%s'); print('sys.path =', sys.path)", importPath));

        // import module 'proc_module.py'
        PyModule procModule = PyModule.importModule("proc_module");
        // Cast the Python module to a Java object of type 'Processor'
        Processor processor = procModule.cast(Processor.class);
        assertNotNull(processor);
        String result;
        result = processor.initialize();
        assertEquals("initialize-1", result);
        result = processor.computeTile(100, 100, new float[100 * 100]);
        assertEquals("computeTile-2-100,100", result);
        result = processor.computeTile(200, 400, new float[100 * 100]);
        assertEquals("computeTile-3-200,400", result);
        result = processor.computeTile(300, 200, new float[100 * 100]);
        assertEquals("computeTile-4-300,200", result);
        result = processor.dispose();
        assertEquals("dispose-5", result);
    }

}
