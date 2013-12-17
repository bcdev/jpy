package org.jpy.python;

/**
 * Native interface for the CPython interpreter.
 *
 * @author Norman Fomferra
 */
public class PyLib {
    static {
        String sharedLibPath = PyConfig.getSharedLibPath();
        //System.out.println("sharedLibPath = " + sharedLibPath);
        System.load(sharedLibPath);
    }

    public static void assertLibInitialized() {
        if (!isInitialized()) {
            throw new RuntimeException("Python interpreter not initialized");
        }
    }

    public static native boolean isInitialized();

    public static native boolean initialize(String[] options, boolean debug);

    public static native void destroy();

    public static native void execScript(String script);

    public static native void decref(long pointer);

    public static native int getIntValue(long pointer);

    public static native double getDoubleValue(long pointer);

    public static native String getStringValue(long pointer);

    public static native Object getObjectValue(long pointer);


    public static native long importModule(String name);

    /**
     * Gets the Python value of a given Python attribute.
     *
     * @param pointer Identifies the Python object which contains the attribute {@code name}.
     * @param name    The attribute name.
     * @return Pointer to a Python object that is the value of the attribute (always a new reference).
     */
    public static native long getAttributeValue(long pointer, String name);

    /**
     * Sets the Python attribute given by {@code name} of the Python object pointed to by {@code pointer}.
     * <p/>
     * Before the Python attribute is set, the Java {@code value} is converted into a corresponding
     * Python object using the optional {@code valueType}.
     * Todo: The {@code value} may also be of type {@code PyObject}.
     * This will be directly translated into the corresponding Python object without conversion.
     *
     * @param pointer   Identifies the Python object which contains the attribute {@code name}.
     * @param name      The attribute name.
     * @param value     The new attribute value.
     * @param valueType Optional type for converting the value to a Python object.
     */
    public static native void setAttributeValue(long pointer, String name, Object value, Class<?> valueType);

    /**
     * Calls a Python callable and returns the resulting Python object.
     * <p/>
     * Before the Python callable is called, the {@code args} array of Java objects is converted into corresponding
     * Python objects.
     * Todo: The {@code args} array may also contain objects of type {@code PyObject}.
     * These will be directly translated into the corresponding Python objects without conversion.
     *
     * @param pointer    Identifies the Python object which contains the callable {@code name}.
     * @param methodCall true, if this is a call of a method of the Python object pointed to by {@code pointer}.
     * @param name       The name of the callable.
     * @param argCount   The argument count (length of the following {@code args} array).
     * @param args       The arguments.
     * @param paramTypes Optional array of parameter types for the conversion of the {@code args} into a Python tuple.
     *                   If not null, it must be an array of length {@code argCount}.
     * @return The resulting Python object (always a new reference).
     */
    public static native long call(long pointer,
                                   boolean methodCall,
                                   String name,
                                   int argCount,
                                   Object[] args,
                                   Class<?>[] paramTypes);

}
