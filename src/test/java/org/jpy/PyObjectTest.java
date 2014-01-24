package org.jpy;

import junit.framework.Assert;
import org.jpy.fixtures.Processor;
import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Ignore;
import org.junit.Test;

import java.io.File;
import java.io.IOException;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;

/**
 * @author Norman Fomferra
 */
public class PyObjectTest {
    @BeforeClass
    public static void setUp() throws Exception {
        assertEquals(false, PyLib.isPythonRunning());
        PyLib.startPython();
        assertEquals(true, PyLib.isPythonRunning());
    }

    @AfterClass
    public static void tearDown() throws Exception {
        PyLib.stopPython();
    }

    @Test(expected = IllegalArgumentException.class)
    public void testNullPointer() throws Exception {
        new PyObject(0);
    }

    @Test
    public void testPointer() throws Exception {
        PyObject pyObject = new PyObject(5120);
        assertEquals(5120, pyObject.getPointer());
    }

    @Test
    public void testToString() throws Exception {
        PyObject pyObject = new PyObject(5120);
        assertEquals("PyObject(pointer=0x1400)", pyObject.toString());
    }

    @Test
    public void testEqualsAndHashCode() throws Exception {
        PyObject pyObject = new PyObject(5120);
        assertEquals(true, pyObject.equals(pyObject));
        assertEquals(true, pyObject.equals(new PyObject(5120)));
        assertEquals(false, pyObject.equals(new PyObject(5121)));
        assertEquals(false, pyObject.equals((Object) 5121L));
        assertEquals(5120, pyObject.hashCode());
    }

    @Test
    public void testCall() throws Exception {
        // Python equivalent:
        //
        // >>> import builtins
        // >>> builtins.max('A', 'Z')
        // 'Z'
        //
        PyModule builtins = PyModule.importModule("builtins");
        PyObject value = builtins.call("max", "A", "Z");
        Assert.assertEquals("Z", value.getStringValue());
    }

    @Test
    public void testGetSetAttributes() throws Exception {
        // Python equivalent:
        //
        // >>> import imp
        // >>> myobj = imp.new_module('myobj')
        // >>> myobj.a = 'Tut tut!'
        // >>> myobj.a
        // 'Tut tut!'
        //
        PyModule imp = PyModule.importModule("imp");
        // Call imp.new_module('') module
        PyObject myobj = imp.call("new_module", "myobj");
        myobj.setAttribute("a", "Tut tut!");
        Assert.assertEquals("Tut tut!", myobj.getAttribute("a", String.class));
        PyObject a = myobj.getAttribute("a");
        Assert.assertEquals("Tut tut!", a.getStringValue());
    }

    @Test
    public void testCreateProxyAndCallSingleThreaded() throws Exception {
        extendPythonPath();
        // import module 'proc_class.py'
        PyModule procModule = PyModule.importModule("proc_class");
        // Instantiate Python object of type 'Processor'
        PyObject procObj = procModule.call("Processor");
        testCallProxySingleThreaded(procObj);
    }

    @Test
    @Ignore
    public void testCreateProxyAndCallMultiThreaded() throws Exception {
        extendPythonPath();
        // import module 'proc_class.py'
        PyModule procModule = PyModule.importModule("proc_class");
        // Instantiate Python object of type 'Processor'
        PyObject procObj = procModule.call("Processor");
        testCallProxyMultiThreaded(procObj);
    }

    static void extendPythonPath() throws IOException {
        // Add module dir to sys.path in order to import file 'proc_class.py'
        String importPath = new File("src/test/python/fixtures").getCanonicalPath();
        //System.out.println("importPath = " + importPath);
        PyLib.execScript(String.format("import sys; sys.path.append('%s'); print('sys.path =', sys.path)", importPath.replace("\\", "\\\\")));
    }

    static void testCallProxySingleThreaded(PyObject procObject) {
        // Cast the Python object to a Java object of type 'Processor'
        Processor processor = procObject.createProxy(Processor.class);
        assertNotNull(processor);

        String result;
        result = processor.initialize();
        assertEquals("initialize", result);
        result = processor.computeTile(100, 100, new float[100 * 100]);
        assertEquals("computeTile-1-100,100", result);
        result = processor.computeTile(200, 100, new float[100 * 100]);
        assertEquals("computeTile-2-200,100", result);
        result = processor.computeTile(100, 200, new float[100 * 100]);
        assertEquals("computeTile-3-100,200", result);
        result = processor.computeTile(200, 200, new float[100 * 100]);
        assertEquals("computeTile-4-200,200", result);
        result = processor.dispose();
        assertEquals("dispose", result);
    }

    static void testCallProxyMultiThreaded(PyObject procObject) {
        // Cast the Python object to a Java object of type 'Processor'
        final Processor processor = procObject.createProxy(Processor.class);
        assertNotNull(processor);

        ProcessorJob[] jobs = new ProcessorJob[]{
                new ProcessorJob(processor, 100, 100),
                new ProcessorJob(processor, 200, 100),
                new ProcessorJob(processor, 100, 200),
                new ProcessorJob(processor, 200, 200),
        };

        Thread[] threads = new Thread[]{
                new Thread(jobs[0]),
                new Thread(jobs[1]),
                new Thread(jobs[2]),
                new Thread(jobs[3]),
        };

        String result;
        result = processor.initialize();
        assertEquals("initialize", result);

        for (Thread thread : threads) {
            thread.start();
        }

        try {
            for (Thread thread : threads) {
                thread.join();
            }
        } catch (InterruptedException e) {
            e.printStackTrace();
        }

        result = jobs[0].result;
        assertEquals("computeTile-1-100,100", result);
        result = jobs[1].result;
        assertEquals("computeTile-2-200,100", result);
        result = jobs[2].result;
        assertEquals("computeTile-3-100,200", result);
        result = jobs[3].result;
        assertEquals("computeTile-4-200,200", result);

        result = processor.dispose();
        assertEquals("dispose", result);

    }

    private static class ProcessorJob implements Runnable {
         final Processor processor;
         int x;
        int y;
        String result;

        public ProcessorJob(Processor processor, int x, int y) {
            this.processor = processor;
            this.x = x;
            this.y = y;
        }

        @Override
        public void run() {
            result = processor.computeTile(x, y, new float[100 * 100]);
        }
    }
}