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
import org.jpy.fixtures.Processor;

import java.io.File;
import java.io.IOException;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.concurrent.*;

import static org.junit.Assert.*;

/**
 * @author Norman Fomferra
 */
public class PyObjectTest extends PyLibTestBase {

    @Override
    protected String[] getExtraPaths() throws Exception {
        return new String[]{new File("src/test/python/fixtures").getCanonicalPath()};
    }

    @Override
    protected int getDiagFlags() {
        return org.jpy.PyLib.Diag.F_ALL;
    }

    @Test(expected = IllegalArgumentException.class)
    public void testNullPointer() throws Exception {
        new PyObject(lib, 0);
    }

    @Test
    public void testPointer() throws Exception {
        long pointer = lib.importModule0("sys");
        PyObject pyObject = new PyObject(lib, pointer);
        assertEquals(pointer, pyObject.getPointer());
    }

    @Test
    public void testToString() throws Exception {
        long pointer = lib.importModule0("sys");
        PyObject pyObject = new PyObject(lib, pointer);
        assertEquals("PyObject(pointer=0x" + Long.toHexString(pointer) + ")", pyObject.toString());
    }

    @Test
    public void testEqualsAndHashCode() throws Exception {
        long pointer1 = lib.importModule0("sys");
        long pointer2 = lib.importModule0("os");
        PyObject pyObject1 = new PyObject(lib, pointer1);
        PyObject pyObject2 = new PyObject(lib, pointer2);
        assertEquals(true, pyObject1.equals(pyObject1));
        assertEquals(true, pyObject1.equals(new PyObject(lib, pointer1)));
        assertEquals(false, pyObject1.equals(pyObject2));
        assertEquals(false, pyObject1.equals(new PyObject(lib, pointer2)));
        assertEquals(false, pyObject1.equals((Object) pointer1));
        assertTrue(0 != pyObject1.hashCode());
        assertTrue(0 != pyObject2.hashCode());
        assertEquals(pyObject1.hashCode(), pyObject1.hashCode());
        assertEquals(pyObject1.hashCode(), new PyObject(lib, pointer1).hashCode());
        assertTrue(pyObject1.hashCode() != pyObject2.hashCode());
    }

    @Test
    public void testExecuteCode_Stmt() throws Exception {
        PyObject pyObject = lib.executeCode("pass", PyInputMode.STATEMENT);
        assertNotNull(pyObject);
        assertNull(pyObject.getObjectValue());
    }

    @Test
    public void testExecuteCode_IntExpr() throws Exception {
        PyObject pyObject = lib.executeCode("7465", PyInputMode.EXPRESSION);
        assertNotNull(pyObject);
        assertEquals(7465, pyObject.getIntValue());
    }

    @Test
    public void testExecuteCode_DoubleExpr() throws Exception {
        PyObject pyObject = lib.executeCode("3.14", PyInputMode.EXPRESSION);
        assertNotNull(pyObject);
        assertEquals(3.14, pyObject.getDoubleValue(), 1e-10);
    }

    @Test
    public void testExecuteCode_StringExpr() throws Exception {
        PyObject pyObject = lib.executeCode("'Hello from Python'", PyInputMode.EXPRESSION);
        assertNotNull(pyObject);
        assertEquals("Hello from Python", pyObject.getStringValue());
    }

    @Test
    public void testExecuteCode_Script() throws Exception {
        HashMap<String, Object> localMap = new HashMap<>();
        PyObject pyVoid = lib.executeCode("" +
                                                    "import jpy\n" +
                                                    "File = jpy.get_type('java.io.File')\n" +
                                                    "f = File('test.txt')",
                                          PyInputMode.SCRIPT,
                                          null,
                                          localMap);
        assertNotNull(pyVoid);
        assertEquals(null, pyVoid.getObjectValue());

/*
        assertNotNull(localMap.get("jpy"));
        assertNotNull(localMap.get("File"));
        assertNotNull(localMap.get("f"));
        assertEquals(PyObject.class, localMap.get("jpy").getClass());
        assertEquals(Class.class, localMap.get("File").getClass());
        assertEquals(File.class, localMap.get("f").getClass());

        assertEquals(new File("test.txt"), localMap.get("f"));
*/
    }

    @Test
    public void testExecuteScript_ErrorExpr() throws Exception {
        try {
            lib.executeCode("[1, 2, 3", PyInputMode.EXPRESSION);
        } catch (RuntimeException e) {
            assertNotNull(e.getMessage());
            assertTrue(e.getMessage().contains("SyntaxError"));
        }
    }

    @Test
    public void testCall() throws Exception {
        // Python equivalent:
        //
        // >>> import builtins
        // >>> builtins.max('A', 'Z')
        // 'Z'
        //
        PyModule builtins;
        try {
            //Python 3.3
            builtins = lib.importModule("builtins");
        } catch (Exception e) {
            //Python 2.7
            builtins = lib.importModule("__builtin__");
        }
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
        PyModule imp = lib.importModule("imp");
        // Call imp.new_module('') module
        PyObject myobj = imp.call("new_module", "myobj");
        myobj.setAttribute("a", "Tut tut!");
        Assert.assertEquals("Tut tut!", myobj.getAttribute("a", String.class));
        PyObject a = myobj.getAttribute("a");
        Assert.assertEquals("Tut tut!", a.getStringValue());
    }

    @Test
    public void testCreateProxyAndCallSingleThreaded() throws Exception {
        //addTestDirToPythonSysPath();
        PyModule procModule = lib.importModule("proc_class");
        PyObject procObj = procModule.call("Processor");
        testCallProxySingleThreaded(procObj);
    }

    // see https://github.com/bcdev/jpy/issues/26
    @Test
    public void testCreateProxyAndCallMultiThreaded() throws Exception {
        //addTestDirToPythonSysPath();
        //lib.Diag.setDiagFlags(lib.Diag.F_ALL);
        PyModule procModule = lib.importModule("proc_class");
        PyObject procObj = procModule.call("Processor");
        testCallProxyMultiThreaded(procObj);
        //lib.Diag.setDiagFlags(lib.Diag.F_OFF);
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
                                                              new ProcessorTask(processor, 200, 200)), 10, TimeUnit.SECONDS);

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

    void addTestDirToPythonSysPath() throws IOException {
        // Add module dir to sys.path in order to import file 'proc_class.py'
        String importPath = new File("src/test/python/fixtures").getCanonicalPath();
        //System.out.println("importPath = " + importPath);
        lib.execScript(String.format("import sys; sys.path.append('%s')", importPath.replace("\\", "\\\\")));
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
