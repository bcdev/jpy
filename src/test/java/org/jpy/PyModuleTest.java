package org.jpy;

import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Ignore;
import org.junit.Test;

import static org.junit.Assert.assertEquals;

/**
 * @author Norman Fomferra
 */
public class PyModuleTest {
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

    @Test
    public void testCreateAndCallProxySingleThreaded() throws Exception {
        PyObjectTest.addTestDirToPythonSysPath();
        PyModule procModule = PyModule.importModule("proc_module");
        PyObjectTest.testCallProxySingleThreaded(procModule);
    }

    // todo - fix https://github.com/bcdev/jpy/issues/26
    @Test
    @Ignore(value = "https://github.com/bcdev/jpy/issues/26")
    public void testCreateAndCallProxyMultiThreaded() throws Exception {
        PyObjectTest.addTestDirToPythonSysPath();
        PyModule procModule = PyModule.importModule("proc_module");
        PyObjectTest.testCallProxyMultiThreaded(procModule);
    }
}
