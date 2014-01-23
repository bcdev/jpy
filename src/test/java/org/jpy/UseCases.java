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
