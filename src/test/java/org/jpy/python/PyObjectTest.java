package org.jpy.python;

import org.junit.Test;
import org.junit.Assert;
import org.jpy.python.PyObject;

import static org.junit.Assert.assertEquals;

/**
 * @author Norman Fomferra
 */
public class PyObjectTest {
    @Test
    public void testIt() throws Exception {
        PyObject pyObject = new PyObject(5120);
        assertEquals(5120, pyObject.getPointer());
        assertEquals("PyObject(pointer=1400)", pyObject.toString());
        assertEquals(true, pyObject.equals(pyObject));
        assertEquals(true, pyObject.equals(new PyObject(5120)));
        assertEquals(false, pyObject.equals(new PyObject(5121)));
        assertEquals(false, pyObject.equals((Object) 5121L));
        assertEquals(5120, pyObject.hashCode());
    }
}
