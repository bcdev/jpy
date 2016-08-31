package org.jpy;

import org.junit.Assert;
import org.junit.Test;

public class LifeCycleTest {

  @Test
  public void testCanStartAndStopWithoutException() {
    PyLib.startPython();
    Assert.assertTrue(PyLib.isPythonRunning());
    PyModule sys = PyModule.importModule("sys");
    Assert.assertNotNull(sys);
    final long firstPointer = sys.getPointer();

    PyLib.stopPython();
    Assert.assertFalse(PyLib.isPythonRunning());

    PyLib.startPython();
    Assert.assertTrue(PyLib.isPythonRunning());

    sys = PyModule.importModule("sys");
    final long secondPointer = sys.getPointer();
    Assert.assertNotEquals(firstPointer, secondPointer);

    PyLib.stopPython();
    Assert.assertFalse(PyLib.isPythonRunning());
  }

}
