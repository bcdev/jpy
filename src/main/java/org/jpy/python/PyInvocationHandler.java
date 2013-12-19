package org.jpy.python;

import java.lang.reflect.InvocationHandler;
import java.lang.reflect.Method;
import java.util.Arrays;

import static org.jpy.python.PyLib.assertInterpreterInitialized;

/**
 * InvocationHandler for the PyInterpreter.
 *
 * @author Norman Fomferra
 */
class PyInvocationHandler implements InvocationHandler {
    private final PyObject pyObject;
    private final boolean methodCall;

    public PyInvocationHandler(PyObject pyObject, boolean methodCall) {
        if (pyObject == null) {
            throw new NullPointerException("pyObject");
        }
        this.pyObject = pyObject;
        this.methodCall = methodCall;
    }

    @Override
    public Object invoke(Object proxyObject, Method method, Object[] args) throws Throwable {
        System.out.printf("invoke: %s(%s)\n", method.getName(), Arrays.toString(args));
        assertInterpreterInitialized();
        return PyLib.callAndReturnValue(
                this.pyObject.getPointer(),
                methodCall,
                method.getName(),
                args != null ? args.length : 0,
                args,
                method.getParameterTypes(),
                method.getReturnType());
    }
}
