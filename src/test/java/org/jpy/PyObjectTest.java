package org.jpy;

import junit.framework.Assert;
import org.jpy.fixtures.Processor;
import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Ignore;
import org.junit.Test;

import java.io.File;
import java.io.IOException;
import java.util.Arrays;
import java.util.List;
import java.util.concurrent.Callable;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.Future;
import java.util.concurrent.TimeUnit;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.fail;

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
        addTestDirToPythonSysPath();
        PyModule procModule = PyModule.importModule("proc_class");
        PyObject procObj = procModule.call("Processor");
        testCallProxySingleThreaded(procObj);
    }

    // todo - fix https://github.com/bcdev/jpy/issues/26
    @Test
    @Ignore(value = "https://github.com/bcdev/jpy/issues/26")
    public void testCreateProxyAndCallMultiThreaded() throws Exception {
        addTestDirToPythonSysPath();
        PyModule procModule = PyModule.importModule("proc_class");
        PyObject procObj = procModule.call("Processor");
        testCallProxyMultiThreaded(procObj);
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

    static void addTestDirToPythonSysPath() throws IOException {
        // Add module dir to sys.path in order to import file 'proc_class.py'
        String importPath = new File("src/test/python/fixtures").getCanonicalPath();
        //System.out.println("importPath = " + importPath);
        PyLib.execScript(String.format("import sys; sys.path.append('%s'); print('sys.path =', sys.path)", importPath.replace("\\", "\\\\")));
    }

    static void testCallProxyMultiThreaded(PyObject procObject) {

        // Cast the Python object to a Java object of type 'Processor'
        final Processor processor = procObject.createProxy(Processor.class);
        assertNotNull(processor);

        String result;
        result = processor.initialize();
        assertEquals("initialize", result);

        int numThreads = 1;
        ExecutorService executorService = Executors.newFixedThreadPool(numThreads);
        List<Future<String>> futures;
        try {
            futures = executorService.invokeAll(Arrays.asList(new ProcessorTask(processor, 100, 100),
                                                              new ProcessorTask(processor, 200, 100),
                                                              new ProcessorTask(processor, 100, 200),
                                                              new ProcessorTask(processor, 200, 200)));
            executorService.awaitTermination(1, TimeUnit.MINUTES);

            result = processor.dispose();
            assertEquals("dispose", result);

            String[] results = new String[]{
                    futures.get(0).get(),
                    futures.get(1).get(),
                    futures.get(2).get(),
                    futures.get(3).get(),
            };

            Arrays.sort(results);

            result = results[0];
            assertEquals("computeTile-1-100,100", result);
            result = results[1];
            assertEquals("computeTile-2-200,100", result);
            result = results[2];
            assertEquals("computeTile-3-100,200", result);
            result = results[3];
            assertEquals("computeTile-4-200,200", result);

        } catch (InterruptedException | ExecutionException e) {
            e.printStackTrace();
            fail(e.getMessage());
        }
    }

    private static class ProcessorTask implements Callable<String> {
        final Processor processor;
        int x;
        int y;

        public ProcessorTask(Processor processor, int x, int y) {
            this.processor = processor;
            this.x = x;
            this.y = y;
        }

        @Override
        public String call() throws Exception {
            return processor.computeTile(x, y, new float[100 * 100]);
        }
    }
}