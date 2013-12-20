package org.jpy.fixtures;

/**
 * Stands for an image data processor.
 *
 * Created by Norman on 19.12.13.
 */
public interface Processor {
    String initialize();

    String computeTile(int w, int h, float[] data);

    String dispose();
}
