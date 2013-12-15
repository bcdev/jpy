package org.jpy.python;

/**
 * Represents a CPython object (of type <code>PyObject *</code>).
 *
 * <p/>
 * <i>Neither used nor implemented yet.</i>
 *
 * @author Norman Fomferra
 */
public class PyModule extends PyObject {
    private final String name;

    PyModule(String name, long pointer) {
        super(pointer);
        this.name = name;
    }

    public String getName() {
        return name;
    }

    @Override
    protected PyInvocationHandler createInvocationHandler() {
        return new PyInvocationHandler(this, false);
    }
}
