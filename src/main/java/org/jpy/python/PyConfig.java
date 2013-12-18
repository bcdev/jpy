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
                try (FileReader reader = new FileReader(proFile)) {
                    properties.load(reader);
                }
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

    public enum OS {
        WINDOWS,
        UNIX,
        MAC_OS,
        SUNOS,
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
