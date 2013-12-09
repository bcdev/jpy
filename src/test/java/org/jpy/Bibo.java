package org.jpy;

/**
 * Used as a test class for the test cases in jpy_test.py
 *
 * @author Norman Fomferra
 */
public class Bibo {
    private final static Bibo PRIMAL_BIBO = new Bibo();

    public static Bibo getPrimalBibo() {
        return PRIMAL_BIBO;
    }

    public Bibo returnBibo(Bibo bibo) {
        return bibo;
    }

    public Bibo[] returnBiboArray(int n) {
        return new Bibo[n];
    }

    public Bibo[] returnBiboArray(Bibo[] bibos) {
        return bibos;
    }

    public String[] getBiboNames() {
        return new String[] {
                "bibo",
                toString(),
                getClass().getName()
        };
    }

    public static double sum(double[] values) {
        double sum = 0;
        for (double value : values) {
            sum += value;
        }
        return sum;
    }
}
