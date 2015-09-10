/*
 * Copyright 2015 Brockmann Consult GmbH
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

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
