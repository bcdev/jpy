package org.jpy.python;

import java.lang.reflect.InvocationHandler;
import java.lang.reflect.Method;

import static org.jpy.python.PyLib.assertInterpreterInitialized;

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
        assertInterpreterInitialized();
        long pointer = PyLib.call(this.pyObject.getPointer(),
                                  isMethod,
                                  method.getName(),
                                  args.length,
                                  args,
                                  method.getParameterTypes());
        if (method.getReturnType().equals(Void.TYPE)) {
            if (pointer != 0) {
                PyLib.decref(pointer);
            }
            return null;
        } else {
            return new PyObject(pointer);
        }
    }
}
