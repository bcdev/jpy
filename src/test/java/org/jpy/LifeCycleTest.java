package org.jpy;

import org.junit.AfterClass;
import org.junit.Assert;
import org.junit.BeforeClass;
import org.junit.Test;

public class LifeCycleTest {

  @Test
  public void testCanStartAndStopWithoutException() {
    PyLib.startPython();
    Assert.assertTrue(PyLib.isPythonRunning());

    PyLib.stopPython();
    Assert.assertFalse(PyLib.isPythonRunning());

    PyLib.startPython();
    Assert.assertTrue(PyLib.isPythonRunning());

    PyLib.stopPython();
    Assert.assertFalse(PyLib.isPythonRunning());
  }

}
