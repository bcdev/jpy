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

import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Test;

import static org.junit.Assert.assertEquals;

/**
 * @author Norman Fomferra
 */
public class PyModuleTest {

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

    @Test
    public void testCreateAndCallProxySingleThreaded() throws Exception {
        PyObjectTest.addTestDirToPythonSysPath();
        PyModule procModule = PyModule.importModule("proc_module");
        PyObjectTest.testCallProxySingleThreaded(procModule);
    }

    // see https://github.com/bcdev/jpy/issues/26
    @Test
    public void testCreateAndCallProxyMultiThreaded() throws Exception {
        PyObjectTest.addTestDirToPythonSysPath();
        PyModule procModule = PyModule.importModule("proc_module");
        PyObjectTest.testCallProxyMultiThreaded(procModule);
    }
}
