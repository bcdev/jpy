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

import junit.framework.Assert;
import org.jpy.fixtures.Processor;
import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Test;

import java.io.File;
import java.io.IOException;
import java.util.Arrays;
import java.util.List;
import java.util.concurrent.*;

import static org.junit.Assert.*;

/**
 * @author Norman Fomferra
 */
public class PyObjectTest {
    @BeforeClass
    public static void setUp() throws Exception {
        System.out.println("PyModuleTest: Current thread: " + Thread.currentThread());

        PyLib.startPython();
        assertEquals(true, PyLib.isPythonRunning());

        PyLib.Diag.setFlags(PyLib.Diag.F_METH);
    }

    @AfterClass
    public static void tearDown() throws Exception {
        PyLib.Diag.setFlags(PyLib.Diag.F_OFF);
        //PyLib.stopPython();
    }

    @Test(expected = IllegalArgumentException.class)
    public void testNullPointer() throws Exception {
        new PyObject(0);
    }

    @Test
    public void testPointer() throws Exception {
        long pointer = PyLib.importModule("sys");
        PyObject pyObject = new PyObject(pointer);
        assertEquals(pointer, pyObject.getPointer());
    }

    @Test
    public void testToString() throws Exception {
        long pointer = PyLib.importModule("sys");
        PyObject pyObject = new PyObject(pointer);
        assertEquals("PyObject(pointer=0x" + Long.toHexString(pointer) + ")", pyObject.toString());
    }

    @Test
    public void testEqualsAndHashCode() throws Exception {
        long pointer1 = PyLib.importModule("sys");
        long pointer2 = PyLib.importModule("os");
        PyObject pyObject1 = new PyObject(pointer1);
        PyObject pyObject2 = new PyObject(pointer2);
        assertEquals(true, pyObject1.equals(pyObject1));
        assertEquals(true, pyObject1.equals(new PyObject(pointer1)));
        assertEquals(false, pyObject1.equals(pyObject2));
        assertEquals(false, pyObject1.equals(new PyObject(pointer2)));
        assertEquals(false, pyObject1.equals((Object) pointer1));
        assertTrue(0 != pyObject1.hashCode());
        assertTrue(0 != pyObject2.hashCode());
        assertEquals(pyObject1.hashCode(), pyObject1.hashCode());
        assertEquals(pyObject1.hashCode(), new PyObject(pointer1).hashCode());
        assertTrue(pyObject1.hashCode() != pyObject2.hashCode());
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

    // see https://github.com/bcdev/jpy/issues/26
    @Test
    public void testCreateProxyAndCallMultiThreaded() throws Exception {
        addTestDirToPythonSysPath();
        //PyLib.Diag.setFlags(PyLib.Diag.F_ALL);
        PyModule procModule = PyModule.importModule("proc_class");
        PyObject procObj = procModule.call("Processor");
        //PyLib.Diag.setFlags(PyLib.Diag.F_ALL);
        testCallProxyMultiThreaded(procObj);
        //PyLib.Diag.setFlags(PyLib.Diag.F_OFF);
    }


    static void testCallProxySingleThreaded(PyObject procObject) {
        // Cast the Python object to a Java object of type 'Processor'
        Processor processor = procObject.createProxy(Processor.class);
        assertNotNull(processor);

        String result;
        result = processor.initialize();
        assertEquals("initialize", result);
        result = processor.computeTile(100, 100, new float[100 * 100]);
        assertEquals("computeTile-100,100", result);
        result = processor.computeTile(200, 100, new float[100 * 100]);
        assertEquals("computeTile-200,100", result);
        result = processor.computeTile(100, 200, new float[100 * 100]);
        assertEquals("computeTile-100,200", result);
        result = processor.computeTile(200, 200, new float[100 * 100]);
        assertEquals("computeTile-200,200", result);
        result = processor.dispose();
        assertEquals("dispose", result);
    }

    static void testCallProxyMultiThreaded(PyObject procObject) {
        testCallProxyMultiThreaded(procObject, Executors.newFixedThreadPool(4));
    }

    private static void testCallProxyMultiThreaded(PyObject procObject, ExecutorService executorService) {
        // Cast the Python object to a Java object of type 'Processor'
        final Processor processor = procObject.createProxy(Processor.class);
        assertNotNull(processor);

        String result;
        result = processor.initialize();
        assertEquals("initialize", result);

        List<Future<String>> futures;
        try {
            futures = executorService.invokeAll(Arrays.asList(new ProcessorTask(processor, 100, 100),
                    new ProcessorTask(processor, 200, 100),
                    new ProcessorTask(processor, 100, 200),
                    new ProcessorTask(processor, 200, 200)));
            //executorService.awaitTermination(1, TimeUnit.MINUTES);

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
            assertEquals("computeTile-100,100", result);
            result = results[1];
            assertEquals("computeTile-100,200", result);
            result = results[2];
            assertEquals("computeTile-200,100", result);
            result = results[3];
            assertEquals("computeTile-200,200", result);

        } catch (InterruptedException | ExecutionException e) {
            e.printStackTrace();
            fail(e.getMessage());
        }
    }

    static void addTestDirToPythonSysPath() throws IOException {
        // Add module dir to sys.path in order to import file 'proc_class.py'
        String importPath = new File("src/test/python/fixtures").getCanonicalPath();
        //System.out.println("importPath = " + importPath);
        PyLib.execScript(String.format("import sys; sys.path.append('%s')", importPath.replace("\\", "\\\\")));
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