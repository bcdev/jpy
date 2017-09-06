package org.jpy;

class Util {
    public enum OS {
        WINDOWS,
        UNIX,
        MAC_OS,
        SUNOS,
    }

    static OS getOS() {
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
