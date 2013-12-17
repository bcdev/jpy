package org.jpy.python;

import junit.framework.Assert;
import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Ignore;
import org.junit.Test;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;

/**
 * @author Norman Fomferra
 */
public class PyObjectTest {
    @BeforeClass
    public static void setUp() throws Exception {
        assertEquals(false, PyLib.isInitialized());
        PyLib.initialize(new String[0], false);
        assertEquals(true, PyLib.isInitialized());
    }

    @AfterClass
    public static void tearDown() throws Exception {
        assertEquals(true, PyLib.isInitialized());
        PyLib.destroy();
        assertEquals(false, PyLib.isInitialized());
    }

    @Test(expected = NullPointerException.class)
    public void testNullPointer() throws Exception {
        new PyObject(0);
    }

    @Test
    public void testNULL() throws Exception {
        assertNotNull(PyObject.NULL);
        assertEquals(0, PyObject.NULL.getPointer());
    }

    @Test
    public void testPointer() throws Exception {
        PyObject pyObject = new PyObject(5120);
        assertEquals(5120, pyObject.getPointer());
    }

    @Test
    public void testToString() throws Exception {
        PyObject pyObject = new PyObject(5120);
        assertEquals("PyObject(pointer=1400)", pyObject.toString());
    }

    @Test
    public void testEqualsAndHashCode() throws Exception {
        PyObject pyObject = new PyObject(5120);
        assertEquals(true, pyObject.equals(pyObject));
        assertEquals(true, pyObject.equals(new PyObject(5120)));
        assertEquals(false, pyObject.equals(new PyObject(5121)));
        assertEquals(false, pyObject.equals((Object) 5121L));
        assertEquals(5120, pyObject.hashCode());
    }

    @Test
    public void testCall() throws Exception {
        PyModule builtins = PyModule.importModule("builtins");
        PyObject value = builtins.call("max", "A", "Z");
        Assert.assertEquals("Z", value.getStringValue());
    }

    @Test
    public void testGetSetAttributes() throws Exception {
        PyModule imp = PyModule.importModule("imp");
        PyObject myobj = imp.call("new_module", "myobj");
        myobj.setAttributeValue("a", "Tut tut!");
        PyObject a = myobj.getAttributeValue("a");
        Assert.assertEquals("Tut tut!", a.getStringValue());
    }


    @Test
    @Ignore
    public void testCast() throws Exception {
        PyModule jpyModule = PyModule.importModule("imp");
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
