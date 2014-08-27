package org.jpy;

/**
 * Replaces System.load().
 */
public class DL {
    public static final int RTLD_LAZY = 0x0001;
    public static final int RTLD_NOW = 0x0002;
    public static final int RTLD_LOCAL = 0x0004;
    public static final int RTLD_GLOBAL = 0x0008;

    public static native long dlopen(String path, int mode);

    public static native int dlclose(long handle);

    public static native String dlerror();

    static {
        try {
            System.loadLibrary("jdl");
        } catch (Throwable t) {
            String jdlLibPath = System.getProperty("jpy.jdlLib");
            if (jdlLibPath != null) {
                System.load(jdlLibPath);
            } else {
                throw new RuntimeException("Failed to load 'jdl' shared library. You can use system property 'jpy.jdlLib' to specify it.", t);
            }
        }
    }
}
