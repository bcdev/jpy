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

import java.io.*;
import java.util.Properties;

/**
 * Provides configuration for {@link org.jpy.PyLib}.
 *
 * @author Norman Fomferra
 * @since 0.7
 */
@SuppressWarnings("WeakerAccess")
public interface PyLibConfig {

    boolean DEBUG = Boolean.getBoolean("jpy.debug");

    String getJpyLibPath();

    String getJdlLibPath();

    String getPythonLibPath();

    class Default implements PyLibConfig {
        private final String pythonLibPath;
        private final String jpyLibPath;
        private final String jdlLibPath;

        public Default(String pythonLibPath, String jpyLibPath, String jdlLibPath) {
            this.pythonLibPath = pythonLibPath;
            this.jpyLibPath = jpyLibPath;
            this.jdlLibPath = jdlLibPath;
        }

        @Override
        public String getPythonLibPath() {
            return pythonLibPath;
        }

        @Override
        public String getJpyLibPath() {
            return jpyLibPath;
        }

        @Override
        public String getJdlLibPath() {
            return jdlLibPath;
        }
    }

    class Props implements PyLibConfig {
        public static final String PYTHON_LIB_KEY = "jpy.pythonLib";
        public static final String JDL_LIB_KEY = "jpy.jdlLib";
        public static final String JPY_LIB_KEY = "jpy.jpyLib";
        public static final String JPY_CONFIG_KEY = "jpy.config";
        public static final String JPY_CONFIG_RESOURCE = "jpyconfig.properties";

        private final Properties properties;

        public Props(Properties properties) {
            this.properties = new Properties(System.getProperties());
            this.properties.putAll(properties);
        }

        @Override
        public String getJpyLibPath() {
            return properties.getProperty(JPY_LIB_KEY);
        }

        @Override
        public String getJdlLibPath() {
            return properties.getProperty(JDL_LIB_KEY);
        }

        @Override
        public String getPythonLibPath() {
            return properties.getProperty(PYTHON_LIB_KEY);
        }

        public static PyLibConfig load(String resourcePath) throws IOException {
            if (resourcePath == null) {
                throw new NullPointerException("resourcePath == null");
            }
            if (DEBUG)
                System.out.printf(String.format("%s: loading configuration resource %s\n", PyLibConfig.class.getName(), resourcePath));
            InputStream stream = Thread.currentThread().getContextClassLoader().getResourceAsStream(resourcePath);
            if (stream == null) {
                throw new FileNotFoundException("resource not found: " + resourcePath);
            }
            return load(stream);
        }

        public static PyLibConfig load(File file) throws IOException {
            if (file == null) {
                throw new NullPointerException("file == null");
            }
            if (DEBUG)
                System.out.printf(String.format("%s: loading configuration file %s\n", PyLibConfig.class.getName(), file));
            try (Reader reader = new FileReader(file)) {
                return load(reader);
            }
        }

        public static PyLibConfig load(InputStream stream) throws IOException {
            if (stream == null) {
                throw new NullPointerException("stream == null");
            }
            try (Reader reader = new InputStreamReader(stream)) {
                return load(reader);
            }
        }

        public static PyLibConfig load(Reader reader) throws IOException {
            if (reader == null) {
                throw new NullPointerException("reader == null");
            }
            Properties properties = new Properties();
            properties.load(reader);
            return new Props(properties);
        }
    }
}



