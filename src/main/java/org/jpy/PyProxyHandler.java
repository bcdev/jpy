/*
 * Copyright 2015 Brockmann Consult GmbH
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
 * use this file except in compliance with the License. You may obtain a copy of
 * the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 * License for the specific language governing permissions and limitations under
 * the License.
 */

package org.jpy;

import java.lang.reflect.InvocationHandler;
import java.lang.reflect.Proxy;
import java.lang.reflect.Method;
import java.util.Arrays;

import static org.jpy.PyLib.assertPythonRuns;

/**
 * The {@code InvocationHandler} for used by the proxy instances created by the
 * {@link PyObject#createProxy(Class)} and {@link PyModule#createProxy(Class)}
 * methods.
 *
 * @author Norman Fomferra
 * @since 0.7
 */
class PyProxyHandler implements InvocationHandler {
    // preloaded Method objects for the methods in java.lang.Object
    private static Method hashCodeMethod;
    
    private static Method equalsMethod;
    
    private static Method toStringMethod;
    static {
        try {
            hashCodeMethod = Object.class.getMethod("hashCode");
            equalsMethod = Object.class.getMethod("equals", new Class[] { Object.class });
            toStringMethod = Object.class.getMethod("toString");
        } catch (NoSuchMethodException e) {
            throw new NoSuchMethodError(e.getMessage());
        }
    }
    
    private final PyObject pyObject;
    
    private final PyLib.CallableKind callableKind;
    
    public PyProxyHandler(PyObject pyObject, PyLib.CallableKind callableKind) {
        if (pyObject == null) {
            throw new NullPointerException("pyObject");
        }
        this.pyObject = pyObject;
        this.callableKind = callableKind;
    }
    
    @Override
    public Object invoke(Object proxyObject, Method method, Object[] args) throws Throwable {
        assertPythonRuns();
        
        if ((PyLib.Diag.getFlags() & PyLib.Diag.F_METH) != 0) {
            System.out.printf("org.jpy.PyProxyHandler: invoke: %s(%s) on pyObject=%s in thread %s\n", method.getName(),
                    Arrays.toString(args), Long.toHexString(this.pyObject.getPointer()), Thread.currentThread());
        }
        String methodName = method.getName();
        Class<?> returnType = method.getReturnType();
        if (method.equals(hashCodeMethod)) {
            return callPythonHash();
        } else if (method.equals(equalsMethod)) {
            if (isProxyEqualsEligible(proxyObject, args[0])) {
                PyObject otherPyObject = proxyGetOtherPyObject(proxyObject, args[0]);
                if (this.pyObject == otherPyObject) {
                    return true;
                } else {
                    args[0] = otherPyObject;
                    if (this.pyObject.hasAttribute("__eq__")) {
                        PyObject eqMethPtr = this.pyObject.getAttribute("__eq__");
                        if (!eqMethPtr.hasAttribute("__func__")) { // Must not
                                                                   // be
                                                                   // implemented
                            return false;
                        }
                    } else {
                        return false;
                    }
                }
            } else {
                return false;
            }
            // It's proxy eligible, but not same object, and __eq__ was
            // implemented
            // so defer to the Python __eq__
            methodName = "__eq__";
        } else if (method.equals(toStringMethod)) {
            methodName = "__str__";
        }
        
        return PyLib.callAndReturnValue(this.pyObject.getPointer(), callableKind == PyLib.CallableKind.METHOD,
                methodName, args != null ? args.length : 0, args, method.getParameterTypes(), returnType);
    }
    
    /**
     * Determines if the two proxy objects implement the same interfaces
     * 
     * @param proxyObject
     * @param otherObject
     * @return
     */
    private boolean isProxyEqualsEligible(Object proxyObject, Object otherObject) {
        boolean result = ((proxyObject.getClass() == otherObject.getClass())
                && (Arrays.deepEquals(proxyObject.getClass().getInterfaces(), otherObject.getClass().getInterfaces())));
        
        return result;
    }
    
    /**
     * Determines the corresponding Python object for the other object passed
     * 
     * @param proxyObject
     * @param otherObject
     * @return
     */
    private PyObject proxyGetOtherPyObject(Object proxyObject, Object otherObject) {
        PyObject result = null;
        InvocationHandler otherProxyHandler = Proxy.getInvocationHandler(otherObject);
        if (otherProxyHandler.getClass() == this.getClass()) {
            PyProxyHandler otherPyProxyHandler = (PyProxyHandler) otherProxyHandler;
            result = otherPyProxyHandler.pyObject;
        }
        
        return result;
    }
    
    /**
     * Calls the Python __hash__ function on the Python object, and returns the
     * last 32 bits of it, since Python hash codes are 64 bits on 64 bit
     * machines.
     * 
     * @return
     */
    private int callPythonHash() {
        long pythonHash = PyLib.callAndReturnValue(this.pyObject.getPointer(), true, "__hash__", 0, null,
                new Class<?>[0], Long.class);
        return (int) pythonHash;
    }
}
