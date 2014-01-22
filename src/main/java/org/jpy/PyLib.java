package org.jpy;

import static org.jpy.PyLibConfig.JPY_LIB_KEY;
import static org.jpy.PyLibConfig.OS;
import static org.jpy.PyLibConfig.PYTHON_LIB_KEY;
import static org.jpy.PyLibConfig.getOS;
import static org.jpy.PyLibConfig.getProperty;

/**
 * Represents the library that provides the Python interpreter (CPython).
 * <p/>
 * When the {@code PyLib} calss is loaded, it reads its configuration from a Java properties file called {@code .jpy}
 * which must exist in the current user's home directory. The confifuration file has been written to this location
 * by installing the Python jpy module using {@code python3 setup.py install --user} on Unix
 * and {@code python setup.py install}) on Windows.
 * <p/>
 * Currently, the following properties are expected in the {@code .jpy} file:
 * <ul>
 * <li>{@code python.lib} - the Python shared library (usually required on Unix only)</li>
 * <li>{@code jpy.lib} - the jpy shared library path (Unix: {@code jpy*.so}, Windows: {@code jpy*.pyd})</li>
 * </ul>
 * <p/>
 * <i>Important note for developers: If you change the signature of any of the native {@code PyLib} methods,
 * you must first run {@code javah} on the compiled class, and then adapt {@code src/main/c/jni/org_jpy_PyLib.c}.</i>
 *
 * @author Norman Fomferra
 * @since 1.0
 */
public class PyLib {

    public enum CallableKind {
        FUNCTION,
        METHOD,
    }

    private static Throwable sharedLibraryProblem;
    private static boolean sharedLibraryLoaded;

    /**
     * Throws a runtime exception if the shared Python library code could not be loaded
     * or if the Python interpreter could not be initialised.
     */
    public static void assertInterpreterInitialized() {
        if (sharedLibraryProblem != null) {
            throw new RuntimeException("PyLib not initialized", sharedLibraryProblem);
        }
        if (!isInterpreterInitialized()) {
            throw new RuntimeException("Python interpreter not initialized");
        }
    }

    public static native boolean isInterpreterInitialized();

    public static native boolean initializeInterpreter();

    public static native void destroyInterpreter();

    public static native int execScript(String script);

    static native void decref(long pointer);

    static native int getIntValue(long pointer);

    static native double getDoubleValue(long pointer);

    static native String getStringValue(long pointer);

    static native Object getObjectValue(long pointer);

    static native long importModule(String name);

    /**
     * Gets the value of a given Python attribute as Python object pointer.
     *
     * @param pointer Identifies the Python object which contains the attribute {@code name}.
     * @param name    The attribute name.
     * @return Pointer to a Python object that is the value of the attribute (always a new reference).
     */
    static native long getAttributeObject(long pointer, String name);

    /**
     * Gets the value of a given Python attribute as Java value.
     *
     * @param pointer   Identifies the Python object which contains the attribute {@code name}.
     * @param name      The attribute name.
     * @param valueType The expected return type.
     * @return A value that represents the converted Python attribute value.
     */
    static native <T> T getAttributeValue(long pointer, String name, Class<T> valueType);

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
    static native <T> void setAttributeValue(long pointer, String name, T value, Class<T> valueType);

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
     *                   If not null, it must be an array of the same length as {@code args}.
     * @return The resulting Python object (always a new reference).
     */
    static native long callAndReturnObject(long pointer,
                                           boolean methodCall,
                                           String name,
                                           int argCount,
                                           Object[] args,
                                           Class<?>[] paramTypes);

    /**
     * Calls a Python callable and returns the a Java Object.
     * <p/>
     * Before the Python callable is called, the {@code args} array of Java objects is converted into corresponding
     * Python objects. The return value of the Python call is converted to a Java object according the the given
     * return type.
     * Todo: The {@code args} array may also contain objects of type {@code PyObject}.
     * These will be directly translated into the corresponding Python objects without conversion.
     *
     * @param pointer    Identifies the Python object which contains the callable {@code name}.
     * @param methodCall true, if this is a call of a method of the Python object pointed to by {@code pointer}.
     * @param name       The name of the callable.
     * @param argCount   The argument count (length of the following {@code args} array).
     * @param args       The arguments.
     * @param paramTypes Optional array of parameter types for the conversion of the {@code args} into a Python tuple.
     *                   If not null, it must be an array of the same length as {@code args}.
     * @param returnType Optional return type.
     * @return The resulting Python object (always a new reference).
     */
    static native <T> T callAndReturnValue(long pointer,
                                           boolean methodCall,
                                           String name,
                                           int argCount,
                                           Object[] args,
                                           Class<?>[] paramTypes,
                                           Class<T> returnType);

    /**
     * Controls output of diagnostic information for debugging.
     */
    public static class Diag {

        static {
            PyLib.loadLib();
        }

        public static final int F_OFF = 0x00;
        public static final int F_TYPE = 0x01;
        public static final int F_METH = 0x02;
        public static final int F_EXEC = 0x04;
        public static final int F_MEM = 0x08;
        public static final int F_ALL = 0xff;

        public static native int getFlags();

        public static native void setFlags(int flags);

    }

    private static void loadLib() {
        if (sharedLibraryLoaded || sharedLibraryProblem != null) {
            return;
        }
        try {
            if (getOS() != OS.WINDOWS) {
                // For PyLib, we load the shared library that was generated for the Python extension module 'jpy'.
                // However, to use 'jpy' from Java we also need the Python shared library to be loaded as well.
                // On Windows, this is done auto-magically, on Linux and Darwin we have to either change 'setup.py'
                // to also include a dependency to the Python shared lib or, as done here, explicitly load it.
                //
                // If the Python shared lib is not found, we get error messages similar to the following:
                // java.lang.UnsatisfiedLinkError: /usr/local/lib/python3.3/dist-packages/jpy.cpython-33m.so:
                //      /usr/local/lib/python3.3/dist-packages/jpy.cpython-33m.so: undefined symbol: PyFloat_Type
                String libPath = getProperty(PYTHON_LIB_KEY, false);
                if (libPath != null) {
                    // E.g. libPath = "/usr/lib/libpython3.3m.so";
                    System.load(libPath);
                }
            }

            String libPath = getProperty(JPY_LIB_KEY, true);
            // E.g. libPath = "/usr/local/lib/python3.3/dist-packages/jpy.cpython-33m.so";
            //System.out.println(JPY_LIB_KEY + " = " + libPath);
            System.load(libPath);
            sharedLibraryProblem = null;
            sharedLibraryLoaded = true;
        } catch (Throwable t) {
            sharedLibraryProblem = t;
            throw t;
        }
    }

    static {
        loadLib();
    }

}


