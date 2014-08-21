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
import org.junit.Test;

/**
 * Some (more complex) tests that represent possible API use cases.
 *
 * @author Norman Fomferra
 */
public class UseCases {

    @Test
    public void modifyPythonSysPath() {

        PyLib.startPython();
        PyModule builtinsMod = PyModule.importBuiltins();

        PyModule sysMod = PyModule.importModule("sys");
        PyObject pathObj = sysMod.getAttribute("path");

        PyObject lenObj1 = builtinsMod.call("len", pathObj);
        pathObj.call("append", "/usr/home/norman/");
        PyObject lenObj2 = builtinsMod.call("len", pathObj);

        int lenVal1 = lenObj1.getIntValue();
        int lenVal2 = lenObj2.getIntValue();
        String[] pathEntries = pathObj.getObjectArrayValue(String.class);

        /////////////////////////////////////////////////

        Assert.assertEquals(lenVal1 + 1, lenVal2);
        Assert.assertEquals(pathEntries.length, lenVal2);
        //for (int i = 0; i < pathEntries.length; i++) {
        //    System.out.printf("pathEntries[%d] = %s%n", i, pathEntries[i]);
        //}

        /////////////////////////////////////////////////

        //PyLib.stopPython();
    }

    @Test
    public void setAndGetGlobalPythonVariables() throws Exception {

        PyLib.startPython();
        PyLib.execScript("paramInt = 123");
        PyLib.execScript("paramStr = 'abc'");
        PyModule mainModule = PyModule.importModule("__main__");
        PyObject paramIntObj = mainModule.getAttribute("paramInt");
        PyObject paramStrObj = mainModule.getAttribute("paramStr");
        int paramIntValue = paramIntObj.getIntValue();
        String paramStrValue = paramStrObj.getStringValue();

        /////////////////////////////////////////////////

        Assert.assertEquals(123, paramIntValue);
        Assert.assertEquals("abc", paramStrValue);

        /////////////////////////////////////////////////

        //PyLib.stopPython();
    }
}
