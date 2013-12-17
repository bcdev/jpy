package org.jpy.python;

import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

/**
 * @author Norman Fomferra
 */
public class PyConfig {

    private static final String[] PYTHON_HOME_PROPS = {
            "org.jpy.python.home"
    };
    private static final String[] PYTHON_HOME_ENVS = {
            "PYTHONHOME",
            "PYTHONPATH",
            "PYTHON_HOME",
    };

    enum OS {
        WINDOWS,
        UNIX,
        MACOS,
        SUNOS,
    }


    public static File[] findPythonHomes() {
        List<File> homes = new ArrayList<>();
        for (String name : PYTHON_HOME_ENVS) {
            collectOrNot(System.getenv(name), homes);
        }
        for (String name : PYTHON_HOME_PROPS) {
            collectOrNot(System.getProperty(name), homes);
        }

        OS os = getOS();

        File[] roots = File.listRoots();
        for (File root : roots) {
            if (os == OS.WINDOWS) {
                searchInto(root, homes);
                searchInto(new File(root, "Program Files"), homes);
                searchInto(new File(root, "Program Files (x86)"), homes);
            } else {
                File usr = new File(root, "usr");
                searchInto(usr, homes);
                searchInto(new File(usr, "local"), homes);
                File opt = new File(root, "opt");
                searchInto(opt, homes);
            }
        }

        return homes.toArray(new File[homes.size()]);
    }

    private static void searchInto(File root, List<File> homes) {
        String[] names = root.list();
        if (names != null) {
            for (String name : names) {
                File file = new File(root, name);
                if (file.isDirectory()) {
                    if (name.toLowerCase().startsWith("python3")) {
                        collect(file, homes);
                    }
                }
            }
        }
    }

    private static void collectOrNot(String value, List<File> homes) {
        if (value != null) {
            File file = new File(value);
            if (file.exists() && file.isDirectory()) {
                collect(file, homes);
            }
        }
    }

    private static void collect(File file, List<File> homes)  {
        try {
            File canonicalFile = file.getCanonicalFile();
            homes.add(canonicalFile);
        } catch (IOException e) {
            // ignored
        }
    }

    static OS getOS() {
        String os = System.getProperty("os.name").toLowerCase();
        if (os.contains("win")) {
            return OS.WINDOWS;
        } else if (os.contains("mac")) {
            return OS.MACOS;
        } else if (os.contains("nix") || os.contains("nux") || os.contains("aix")) {
            return OS.UNIX;
        } else if (os.contains("sunos")) {
            return OS.SUNOS;
        }
        return null;
    }
}
