package org.jpy.python;

import java.io.File;
import java.io.FileReader;
import java.io.IOException;
import java.util.Properties;

/**
 * @author Norman Fomferra
 */
public class PyConfig {
    public static final String JPY_LIB_KEY = "jpy.lib";

    public static String getSharedLibPath() {
        String property = System.getProperty(JPY_LIB_KEY);
        if (property != null) {
            return property;
        }
        File proFile = new File(System.getProperty("user.home"), ".jpy");
        if (proFile.exists()) {
            Properties properties = new Properties();
            try {
                properties.load(new FileReader(proFile));
            } catch (IOException e) {
                // ignore silently
            }
            property = properties.getProperty(JPY_LIB_KEY);
            if (property != null) {
                return property;
            }
        }

        throw new RuntimeException("missing system property '" + JPY_LIB_KEY + "'");
    }
}
