/*
 * Copyright (C) 2014 Brockmann Consult GmbH
 *
 * This program is free software; you can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation; either version 3 of the License,
 * or (at your option) any later version. This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
 * PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program; if not, see
 * http://www.gnu.org/licenses/
 */

package org.jpy;

import java.lang.reflect.InvocationHandler;
import java.lang.reflect.Method;
import java.util.Arrays;

import static org.jpy.PyLib.assertPythonRuns;

/**
 * The {@code InvocationHandler} for used by the proxy instances created by the
 * {@link PyObject#createProxy(Class)} and {@link PyModule#createProxy(Class)} methods.
 *
 * @author Norman Fomferra
 * @since 1.0
 */
class PyProxyHandler implements InvocationHandler {
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

        System.out.printf("PyProxyHandler: invoke: %s(%s) in thread %s\n", method.getName(), Arrays.toString(args), Thread.currentThread());

        return PyLib.callAndReturnValue(
                this.pyObject.getPointer(),
                callableKind == PyLib.CallableKind.METHOD,
                method.getName(),
                args != null ? args.length : 0,
                args,
                method.getParameterTypes(),
                method.getReturnType());
    }
}
