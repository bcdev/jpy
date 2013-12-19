package org.jpy.python;

/**
* Created by Norman on 19.12.13.
*/
public interface Proc {
    String initialize();

    String computeTile(int w, int h, float[] data);

    String dispose();
}
