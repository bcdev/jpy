/*
 * Copyright (C) 2014 Brockmann Consult GmbH
 *
 * This program is free software; you can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation; either version 3 of the License,
 * or (at your option) any later version. This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
 * PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program; if not, see
 * http://www.gnu.org/licenses/
 */

package org.jpy;

import java.io.File;
import java.util.ArrayList;

import static org.jpy.PyLibConfig.JPY_LIB_KEY;
import static org.jpy.PyLibConfig.OS;
import static org.jpy.PyLibConfig.PYTHON_LIB_KEY;
import static org.jpy.PyLibConfig.getOS;
import static org.jpy.PyLibConfig.getProperty;

/**
 * Represents the library that provides the Python interpreter (CPython).
 * <p/>
 * When the {@code PyLib} class is loaded, it reads its configuration from a Java properties file called {@code .jpy}
 * which must exist in the current user's home directory. The configuration file has been written to this location
 * by installing the Python jpy module using {@code python3 setup.py install --user} on Unix
 * and {@code python setup.py install}) on Windows.
 * <p/>
 * Currently, the following properties are recognised in the {@code .jpy} file:
 * <ul>
 * <li>{@code python.lib} - the Python shared library (usually required on Unix only)</li>
 * <li>{@code jpy.lib} - the jpy shared library path for Python (Unix: {@code jpy*.so}, Windows: {@code jpy*.pyd})</li>
 * </ul>
 * <p/>
 * jpy API clients should first call {@link #isPythonRunning()} in order to check if a Python interpreter is already available.
 * If not, {@link #startPython(String...)} must be called before any other jpy API is used.
 * <p/>
 * <i>Important note for developers: If you change the signature of any of the native {@code PyLib} methods,
 * you must first run {@code javah} on the compiled class, and then adapt {@code src/main/c/jni/org_jpy_PyLib.c}.</i>
 *
 * @author Norman Fomferra
 * @since 0.7
 */
public class PyLib {

    private static final boolean DEBUG = Boolean.getBoolean("jpy.debug");
    private static String dllFilePath;
    private static Throwable dllProblem;
    private static boolean dllLoaded;

    /**
     * The kind of callable Python objects.
     */
    public enum CallableKind {
        /**
         * Function call without the Python {@code self} as first argument.
         */
        FUNCTION,
        /**
         * An instance method call with the Python {@code self} (= the instance) as first argument.
         */
        METHOD,
    }

    /**
     * Controls output of diagnostic information for debugging.
     */
    @SuppressWarnings("UnusedDeclaration")
    public static class Diag {

        static {
            PyLib.loadLib();
        }

        /**
         * Print no diagnostic information at all.
         */
        public static final int F_OFF = 0x00;
        /**
         * Print diagnostic information while Java types are resolved.
         */
        public static final int F_TYPE = 0x01;
        /**
         * Print diagnostic information while Java methods overloads are selected.
         */
        public static final int F_METH = 0x02;
        /**
         * Print diagnostic information when code execution flow is passed from Java to Python or the other way round.
         */
        public static final int F_EXEC = 0x04;
        /**
         * Print diagnostic information about memory allocation/deallocation.
         */
        public static final int F_MEM = 0x08;
        /**
         * Print diagnostic information usage of the Java VM Invocation API.
         */
        public static final int F_JVM = 0x10;
        /**
         * Print diagnostic information if erroneous states are detected in the jpy Python module.
         */
        public static final int F_ERR = 0x20;
        /**
         * Print any diagnostic information.
         */
        public static final int F_ALL = 0xff;

        /**
         * @return the current diagnostic flags.
         */
        public static native int getFlags();

        /**
         * Sets the current diagnostic flags.
         *
         * @param flags the current diagnostic flags.
         */
        public static native void setFlags(int flags);

        private Diag() {
        }
    }

    @SuppressWarnings("UnusedDeclaration")
    public static String getDllFilePath() {
        return dllFilePath;
    }


    /**
     * Throws a runtime exception if Python interpreter is not running. Possible reasons for this are
     * <ul>
     * <li>You have not called {@link #startPython(String...)} yet.</li>
     * <li>The Python shared library code for the Python interpreter could not be found or could not be be loaded.</li>
     * <li>The Python shared library code for the Python 'jpy' module could not be found or could not be be loaded.</li>
     * <li>The Python interpreter could not be initialised.</li>
     * </ul>
     */
    public static void assertPythonRuns() {
        if (dllProblem != null) {
            throw new RuntimeException("PyLib not initialized", dllProblem);
        }
        if (!isPythonRunning()) {
            throw new RuntimeException("PyLib not initialized");
        }
    }

    /**
     * @return {@code true} if the Python interpreter is running and the the 'jpy' module has been loaded.
     */
    public static native boolean isPythonRunning();

    /**
     * Starts the Python interpreter. It does the following:
     * <ol>
     * <li>Initializes the Python interpreter, if not already running.</li>
     * <li>Prepends any given extra paths to Python's 'sys.path' (e.g. so that 'jpy' can be loaded from isolated directories).</li>
     * <li>Imports the 'jpy' extension module, if not already done.</li>
     * </ol>
     *
     * @param extraPaths List of paths that will be prepended to Python's 'sys.path'.
     * @throws RuntimeException if Python could not be started or if the 'jpy' extension module could not be loaded.
     */
    public static void startPython(String... extraPaths) {

        ArrayList<File> dirList = new ArrayList<>(1 + extraPaths.length);

        File moduleDir = new File(dllFilePath).getParentFile();
        if (moduleDir != null) {
            dirList.add(moduleDir.getAbsoluteFile());
        }

        for (String extraPath : extraPaths) {
            File extraDir = new File(extraPath).getAbsoluteFile();
            if (!dirList.contains(extraDir)) {
                dirList.add(extraDir);
            }
        }

        extraPaths = new String[dirList.size()];
        for (int i = 0; i < dirList.size(); i++) {
            extraPaths[i] = dirList.get(i).getPath();
        }

        if (DEBUG) {
            System.out.println("org.jpy.PyLib: Starting Python with " + extraPaths.length + " extra module path(s):");
            for (String path : extraPaths) {
                System.out.println("org.jpy.PyLib:  " + path);
            }
            Diag.setFlags(Diag.F_EXEC);
        }

        startPython0(extraPaths);
    }

    static native boolean startPython0(String... paths);

    /**
     * @return The Python interpreter version string.
     */
    public static native String getPythonVersion();

    /**
     * Stops the Python interpreter.
     */
    public static native void stopPython();

    public static native int execScript(String script);

    static native void incRef(long pointer);

    static native void decRef(long pointer);

    static native int getIntValue(long pointer);

    static native double getDoubleValue(long pointer);

    static native String getStringValue(long pointer);

    static native Object getObjectValue(long pointer);

    static native <T> T[] getObjectArrayValue(long pointer, Class<T> itemType);

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
     * The {@code value} may also be of type {@code PyObject}.
     * In this case, it will be directly translated into the corresponding Python object without conversion.
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
     * The {@code args} array may also contain objects of type {@code PyObject}.
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
     * The {@code args} array may also contain objects of type {@code PyObject}.
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

    private static void loadLib() {
        if (dllLoaded || dllProblem != null) {
            return;
        }
        try {
            String pythonLibPath = getProperty(PYTHON_LIB_KEY, false);

            if (pythonLibPath != null && getOS() != OS.WINDOWS) {
                // For PyLib, we load the shared library that was generated for the Python extension module 'jpy'.
                // However, to use 'jpy' from Java we also need the Python shared library to be loaded as well.
                // On Windows, this is done auto-magically, on Linux and Darwin we have to either change 'setup.py'
                // to also include a dependency to the Python shared lib or, as done here, explicitly load it.
                //
                // If the Python shared lib is not found, we get error messages similar to the following:
                // java.lang.UnsatisfiedLinkError: /usr/local/lib/python3.3/dist-packages/jpy.cpython-33m.so:
                //      /usr/local/lib/python3.3/dist-packages/jpy.cpython-33m.so: undefined symbol: PyFloat_Type
                if (DEBUG) System.out.println("org.jpy.PyLib: DL.dlopen(\"" + pythonLibPath + "\", DL.RTLD_GLOBAL + DL.RTLD_LAZY");

                long handle = DL.dlopen(pythonLibPath, DL.RTLD_GLOBAL + DL.RTLD_LAZY);
                if (handle == 0) {
                    String dlerror = DL.dlerror();
                    String message = "Failed to load Python shared library. Use system property 'python.lib' to specify it.";
                    if (dlerror != null) {
                        message += " Error: " + dlerror;
                    }
                    throw new RuntimeException(message);
                }
            }


            // E.g. dllFilePath = "/usr/local/lib/python3.3/dist-packages/jpy.cpython-33m.so";
            dllFilePath = getProperty(JPY_LIB_KEY, true);
            dllFilePath = new File(dllFilePath).getAbsolutePath();

            if (DEBUG) System.out.println("org.jpy.PyLib: System.load(\"" + dllFilePath + "\"");
            //if (DEBUG) System.out.println("org.jpy.PyLib: context class loader: " + Thread.currentThread().getContextClassLoader());
            //if (DEBUG) System.out.println("org.jpy.PyLib: class class loader:   " + PyLib.class.getClassLoader());

            System.load(dllFilePath);
            dllProblem = null;
            dllLoaded = true;
        } catch (Throwable t) {
            dllProblem = t;
            throw t;
        }
    }

    private PyLib() {
    }

    static {
        if (DEBUG) System.out.println("org.jpy.PyLib: entered static initializer");
        loadLib();
        if (DEBUG) System.out.println("org.jpy.PyLib: exited static initializer");
    }
}


