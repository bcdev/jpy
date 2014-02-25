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

import org.junit.Test;

/**
 * @author Norman Fomferra
 */
public class UseCases {

    @Test
    public void modifyPythonSysPath() {
        PyLib.startPython();

        PyModule builtins = PyModule.importModule("builtins");

        PyModule sys = PyModule.importModule("sys");
        PyObject path = sys.getAttribute("path");
        path.call("append", "/usr/home/norman/");
        //String value = path.getStringValue();

        PyObject len = builtins.call("len", path);
        int intValue = len.getIntValue();

        String[] objectArrayValue = path.getObjectArrayValue(String.class);

        for (String s : objectArrayValue) {
            System.out.println("s = " + s);
        }

        System.out.println("intValue = " + intValue);
        //System.out.println("value = " + value);

        //PyLib.stopPython();
    }

    @Test
    public void executeScriptAsConfig() throws Exception {
        PyLib.startPython();
        PyModule module = PyModule.importModule("__main__");
        System.out.println("module = " + module);
        PyLib.execScript("a = 1234");
        PyObject a = module.getAttribute("a");
        System.out.println("a = " + a);
        int intValue = a.getIntValue();
        System.out.println("intValue = " + intValue);


    }
}
