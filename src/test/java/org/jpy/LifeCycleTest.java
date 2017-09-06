package org.jpy;

import org.junit.Assert;
import org.junit.Test;

public class LifeCycleTest extends PyLibTestBase {
    private static final boolean ON_WINDOWS = System.getProperty("os.name").toLowerCase().contains("windows");

    @Test
    public void testCanStartAndStopWithoutException() {
        lib.startPython();
        Assert.assertTrue(lib.isPythonRunning());
        PyModule sys1 = lib.importModule("sys");
        Assert.assertNotNull(sys1);
        final long sys1Pointer = sys1.getPointer();

        lib.stopPython();
        if (!ON_WINDOWS) {
            Assert.assertFalse(lib.isPythonRunning());
        }

        lib.startPython();
        Assert.assertTrue(lib.isPythonRunning());

        PyModule sys2 = lib.importModule("sys");
        Assert.assertNotNull(sys2);
        final long sys2Pointer = sys2.getPointer();

        if (!ON_WINDOWS) {
            Assert.assertNotEquals(sys1Pointer, sys2Pointer);
        }

        lib.stopPython();
        if (!ON_WINDOWS) {
            Assert.assertFalse(lib.isPythonRunning());
        }
    }

    @Override
    protected void startInterpreter() {
    }

    @Override
    protected void stopInterpreter() {
    }
}
