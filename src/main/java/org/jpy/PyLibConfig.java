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
import java.io.FileReader;
import java.io.IOException;
import java.util.Properties;

/**
 * Provides configuration for {@link org.jpy.PyLib}.
 *
 * @author Norman Fomferra
 * @since 1.0
 */
class PyLibConfig {
    public static final File JPY_CONFIG_FILE = new File(System.getProperty("user.home"), ".jpy");
    public static final String PYTHON_LIB_KEY = "python.lib";
    public static final String JPY_LIB_KEY = "jpy.lib";

    public enum OS {
        WINDOWS,
        UNIX,
        MAC_OS,
        SUNOS,
    }

    private static final Properties properties = new Properties();

    static {
        if (JPY_CONFIG_FILE.exists()) {
            try {
                try (FileReader reader = new FileReader(JPY_CONFIG_FILE)) {
                    properties.load(reader);
                }
            } catch (IOException e) {
                System.err.printf("%s: failed to read from '%s'\n", PyLibConfig.class.getName(), JPY_CONFIG_FILE);
                //e.printStackTrace(System.err);
            }
        }
    }

    public static String getProperty(String key, boolean mustHave) {
        // System properties overwrite .jpy properties
        String property = System.getProperty(key);
        if (property != null) {
            return property;
        }
        property = properties.getProperty(key);
        if (property == null && mustHave) {
            throw new RuntimeException("missing configuration property '" + key + "'");
        }
        return property;
    }

    public static OS getOS() {
        String os = System.getProperty("os.name").toLowerCase();
        if (os.contains("win")) {
            return OS.WINDOWS;
        } else if (os.contains("nix") || os.contains("nux") || os.contains("aix")) {
            return OS.UNIX;
        } else if (os.contains("mac")) {
            return OS.MAC_OS;
        } else if (os.contains("sunos")) {
            return OS.SUNOS;
        }
        return null;
    }
}
