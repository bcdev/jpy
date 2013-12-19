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
        assertEquals(false, PyLib.isInterpreterInitialized());
        PyLib.initializeInterpreter(new String[0], false);
        assertEquals(true, PyLib.isInterpreterInitialized());
    }

    @AfterClass
    public static void tearDown() throws Exception {
        PyLib.destroyInterpreter();
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
        // Python equivalent:
        //
        // >>> import builtins
        // >>> builtins.max('A', 'Z')
        // 'Z'
        //
        PyModule builtins = PyModule.importModule("builtins");
        PyObject value = builtins.call("max", "A", "Z");
        Assert.assertEquals("Z", value.getStringValue());
    }

    @Test
    public void testGetSetAttributes() throws Exception {
        // Python equivalent:
        //
        // >>> import imp
        // >>> myobj = imp.new_module('myobj')
        // >>> myobj.a = 'Tut tut!'
        // >>> myobj.a
        // 'Tut tut!'
        //
        PyModule imp = PyModule.importModule("imp");
        // Call imp.new_module('') module
        PyObject myobj = imp.call("new_module", "myobj");
        myobj.setAttributeValue("a", "Tut tut!");
        Assert.assertEquals("Tut tut!", myobj.getAttributeValue("a", String.class));
        PyObject a = myobj.getAttributeObject("a");
        Assert.assertEquals("Tut tut!", a.getStringValue());
    }


    @Test
    public void testCast() throws Exception {
        PyModule procModule = PyModule.importModule("proc_class");

        PyObject procObj = procModule.call("Proc");
        Proc proc = procObj.cast(Proc.class);
        assertNotNull(proc);
        String result;
        result = proc.initialize();
        assertEquals("initialize-1", result);
        result = proc.computeTile(100, 100, new float[100 * 100]);
        assertEquals("computeTile-2", result);
        result = proc.computeTile(200, 100, new float[100 * 100]);
        assertEquals("computeTile-3", result);
        result = proc.computeTile(300, 100, new float[100 * 100]);
        assertEquals("computeTile-4", result);
        result = proc.dispose();
        assertEquals("dispose-5", result);
    }
}