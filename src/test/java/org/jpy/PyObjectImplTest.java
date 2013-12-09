package org.jpy;

import org.junit.Test;
import org.junit.Assert;

/**
 * @author Norman Fomferra
 */
public class PyObjectImplTest {
    @Test
    public void testIt() throws Exception {
        PyObject pyObject = new PyObjectImpl(5120);
        Assert.assertEquals(5120, pyObject.getPointer());
        Assert.assertEquals("PyObjectImpl(pointer=1400)", pyObject.toString());
        Assert.assertEquals(true, pyObject.equals(pyObject));
        Assert.assertEquals(true, pyObject.equals(new PyObjectImpl(5120)));
        Assert.assertEquals(false, pyObject.equals(new PyObjectImpl(5121)));
        Assert.assertEquals(false, pyObject.equals((Object) 5121L));
        Assert.assertEquals(5120, pyObject.hashCode());
    }
}
