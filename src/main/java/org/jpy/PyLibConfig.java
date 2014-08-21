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
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.Reader;
import java.util.Properties;
import java.util.Set;

/**
 * Provides configuration for {@link org.jpy.PyLib}.
 *
 * @author Norman Fomferra
 * @since 0.7
 */
class PyLibConfig {
    /**
     * @deprecated No longer used.
     */
    @Deprecated
    public static final File JPY_CONFIG_FILE = new File(System.getProperty("user.home"), ".jpy");
    public static final String PYTHON_LIB_KEY = "jpy.pythonLib";
    public static final String JPY_LIB_KEY = "jpy.jpyLib";
    public static final String JDL_LIB_KEY = "jpy.jdlLib";
    public static final String JPY_CONFIG_KEY = "jpy.config";
    public static final String JPY_CONFIG_RESOURCE = "jpyconfig.properties";

    public enum OS {
        WINDOWS,
        UNIX,
        MAC_OS,
        SUNOS,
    }

    private static final Properties properties = new Properties();


    static {
        InputStream stream = Thread.currentThread().getContextClassLoader().getResourceAsStream(JPY_CONFIG_RESOURCE);
        if (stream != null) {
            loadConfig(stream, JPY_CONFIG_RESOURCE);
        }
        String path = System.getProperty(JPY_CONFIG_KEY);
        if (path != null) {
            File file = new File(path);
            if (file.isFile()) {
                loadConfig(file);
            }
        }
        File file = new File(JPY_CONFIG_RESOURCE).getAbsoluteFile();
        if (file.isFile()) {
            loadConfig(file);
        }
    }

    private static void loadConfig(InputStream stream, String name) {
        try {
            System.out.printf(String.format("%s: loading configuration resource %s\n", PyLibConfig.class.getName(), name));
            try (Reader reader = new InputStreamReader(stream)) {
                loadConfig(reader);
            }
        } catch (IOException e) {
            System.err.printf("%s: %s: %s\n", PyLibConfig.class.getName(), name, e.getMessage());
        }
    }

    private static void loadConfig(File file) {
        try {
            System.out.printf(String.format("%s: loading configuration file %s\n", PyLibConfig.class.getName(), file));
            try (Reader reader = new FileReader(file)) {
                loadConfig(reader);
            }
        } catch (IOException e) {
            System.err.printf("%s: %s: %s\n", PyLibConfig.class.getName(), file, e.getMessage());
        }
    }

    private static void loadConfig(Reader reader) throws IOException {
        properties.load(reader);
        Set<String> propertyNames = properties.stringPropertyNames();
        for (String propertyName : propertyNames) {
            String propertyValue = properties.getProperty(propertyName);
            if (propertyValue != null) {
                System.setProperty(propertyName, propertyValue);
            }
        }
    }

    public static Properties getProperties() {
        return new Properties(properties);
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
