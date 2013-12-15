package org.jpy.python;

/**
 * Represents the CPython interpreter.
 *
 * @author Norman Fomferra
 */
public class PyLib {
    static {
        System.loadLibrary("jpy");
    }

    public static native boolean isInitialized();

    public static native boolean initialize(String[] options, boolean debug);

    public static native void destroy();

    public static native void decref(long pointer);

    public static native long importModule(String name);

    public static native long getAttributeValue(long pointer, String name);

    public static native void setAttributeValue(long pointer, String name, long value);

    public static native void execScript(String script);

    public static native Object call(long pointer, boolean methodCall, String name, Object[] args);

    public static native Object callWithTypeInfo(long pointer, boolean methodCall, String name, Class<?>[] parameterTypes, Class<?> returnType, Object[] args);

    public static void assertLibInitialized() {
        if (!isInitialized()) {
            throw new RuntimeException("Python interpreter not initialized");
        }
    }
}
