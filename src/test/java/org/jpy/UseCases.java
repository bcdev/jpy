package org.jpy;

import org.junit.Test;

/**
 * @author Norman Fomferra
 */
public class UseCases {
    @Test
    public void testIt() {
        PyLib.startPython();

        PyModule builtins = PyModule.importModule("builtins");

        PyModule sys = PyModule.importModule("sys");
        PyObject path = sys.getAttribute("path");
        path.call("append", "/usr/home/norman/");
        //String value = path.getStringValue();

        PyObject len = builtins.call("len", path);
        int intValue = len.getIntValue();

        String[] objectArrayValue = path.getObjectArrayValue(String.class);

        for (int i = 0; i < objectArrayValue.length; i++) {
            String s = objectArrayValue[i];
            System.out.println("s = " + s);
        }

        System.out.println("intValue = " + intValue);
        //System.out.println("value = " + value);
    }
}
