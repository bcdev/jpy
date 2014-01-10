package org.jpy.fixtures;

/**
 * @author Norman Fomferra
 */
@SuppressWarnings("UnusedDeclaration")
public class ConstructorOverloadTestFixture {
    String state;

    public ConstructorOverloadTestFixture() {
        initState();
    }

    public ConstructorOverloadTestFixture(int a) {
        initState(a);
    }

    public ConstructorOverloadTestFixture(int a, int b) {
        initState(a, b);
    }

    public ConstructorOverloadTestFixture(float a) {
        initState(a);
    }

    public ConstructorOverloadTestFixture(float a, float b) {
        initState(a, b);
    }

    public ConstructorOverloadTestFixture(int a, float b) {
        initState(a, b);
    }

    public ConstructorOverloadTestFixture(float a, int b) {
        initState(a, b);
    }

    public String getState() {
        return state;
    }

    private void initState(Object... args) {
        state = MethodOverloadTestFixture.stringifyArgs(args);
    }
}
