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

import java.io.File;
import java.net.URI;
import java.security.CodeSource;

import static org.junit.Assert.*;

public class PyLibWithSysPathTest {

    @BeforeClass
    public static void setUpClass() throws Exception {

        CodeSource codeSource = PyLibWithSysPathTest.class.getProtectionDomain().getCodeSource();
        if (codeSource == null) {
            System.out.println(PyLibWithSysPathTest.class + " not run: no code source found");
            return;
        }
        URI codeSourceLocation = codeSource.getLocation().toURI();
        System.out.println(PyLibWithSysPathTest.class + ": code source: " + codeSourceLocation);
        File codeSourceDir = new File(codeSourceLocation);
        if (!codeSourceDir.isDirectory()) {
            System.out.println(PyLibWithSysPathTest.class + " not run: code source is not a directory: " + codeSourceLocation);
            return;
        }

        File pymodulesDir = new File(codeSourceDir, "pymodules");
        //assertFalse(PyLib.isPythonRunning());
        System.out.println("PyLibWithSysPathTest: starting Python with 'sys.path' extension: " + pymodulesDir);
        PyLib.startPython(pymodulesDir.getPath());
        //PyLib.startPython("x");
        assertTrue(PyLib.isPythonRunning());
    }

    @AfterClass
    public static void tearDownClass() throws Exception {
        //PyLib.stopPython();
    }

    @Test
    public void testLoadModule() throws Exception {
        PyModule pyModule = PyModule.importModule("mod_1");
        assertNotNull(pyModule);
        PyObject pyAnswer = pyModule.getAttribute("answer");
        assertNotNull(pyAnswer);
        assertEquals(42, pyAnswer.getIntValue());
    }
}
