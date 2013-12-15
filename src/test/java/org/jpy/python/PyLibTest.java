package org.jpy.python;

import org.junit.Test;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;

public class PyLibTest {
    @Test
    public void testIt() throws Exception {
        assertEquals(false, PyLib.isInitialized());
        PyLib.initialize(new String[0], true);
        assertEquals(true, PyLib.isInitialized());

        long jpyPointer = PyLib.importModule("jpy");
        assertTrue(jpyPointer != 0);

        long jTypePointer = PyLib.getAttributeValue(jpyPointer, "JType");
        assertTrue(jTypePointer != 0);

        PyLib.setAttributeValue(jpyPointer, "JT", jTypePointer);
        PyLib.call(0, true, "f", new Object[]{1, 2, 3});
        PyLib.callWithTypeInfo(0, true, "f", new Class[0], null, new Object[]{1, 2, 3});

        PyLib.execScript("print('Hello Python!')");
        PyLib.execScript("print('Hello Python 2!')");

        PyModule jpyModule = new PyModule("jpy", jpyPointer);
        Wraw wraw = jpyModule.cast(Wraw.class);
        wraw.initialize();
        wraw.computeTile(100, 100, new float[100 * 100]);
        wraw.computeTile(100, 100, new float[100 * 100]);
        wraw.computeTile(100, 100, new float[100 * 100]);
        wraw.dispose();
    }

    public static interface Wraw {
        void initialize();

        void computeTile(int w, int h, float[] data);

        void dispose();
    }
}
