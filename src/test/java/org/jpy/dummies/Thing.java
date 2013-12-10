package org.jpy.dummies;

/**
 * @author Norman Fomferra
 */
public class Thing {
    private int value;

    public Thing() {
    }

    public Thing(int value) {
        this.value = value;
    }

    public int getValue() {
        return value;
    }

    public void setValue(int value) {
        this.value = value;
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;
        Thing thing = (Thing) o;
        return value == thing.value;
    }

    @Override
    public int hashCode() {
        return value;
    }

    @Override
    public String toString() {
        return "Thing[value=" + value + "]";
    }
}
