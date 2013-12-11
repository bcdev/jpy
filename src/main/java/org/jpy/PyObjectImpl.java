package org.jpy;

/**
 * Wraps a CPython (of type <code>PyObject *</code>).
 *
 * <p/>
 * <i>Neither used nor implemented yet.</i>
 *
 * @author Norman Fomferra
 */
public class PyObjectImpl implements PyObject {
    private final long pointer;

    public PyObjectImpl(long pointer) {
        this.pointer = pointer;
    }

    @Override
    public long getPointer() {
        return pointer;
    }

    @Override
    public String toString() {
        return String.format("PyObjectImpl(pointer=%s)", Long.toHexString(pointer));
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) {
            return true;
        }
        if (!(o instanceof PyObjectImpl)) {
            return false;
        }
        PyObjectImpl pyObject = (PyObjectImpl) o;
        return pointer == pyObject.pointer;

    }

    @Override
    public int hashCode() {
        return (int) (pointer ^ (pointer >>> 32));
    }
}
