package org.jpy.python;

import java.lang.reflect.InvocationHandler;
import java.lang.reflect.Method;

import static org.jpy.python.PyLib.assertLibInitialized;

/**
 * InvocationHandler for the PyInterpreter.
 *
 * @author Norman Fomferra
 */
class PyInvocationHandler implements InvocationHandler {
    private PyObject pyObject;
    private boolean isMethod;

    public PyInvocationHandler(PyObject pyObject, boolean isMethod) {
        this.pyObject = pyObject;
        this.isMethod = isMethod;
    }

    @Override
    public Object invoke(Object proxyObject, Method method, Object[] args) throws Throwable {
        System.out.println("method = " + method.getName());
        assertLibInitialized();
        return PyLib.callWithTypeInfo(
                this.pyObject.getPointer(),
                isMethod,
                method.getName(),
                method.getParameterTypes(),
                method.getReturnType().equals(Void.TYPE) ? null : method.getReturnType(),
                args);
    }
}
