/*
 * Copyright 2015 Brockmann Consult GmbH
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package org.jpy;

import java.lang.reflect.InvocationHandler;
import java.lang.reflect.Method;
import java.util.Arrays;

/**
 * The {@code InvocationHandler} for used by the proxy instances created by the
 * {@link PyObject#createProxy(Class)} and {@link PyModule#createProxy(Class)} methods.
 *
 * @author Norman Fomferra
 * @since 0.7
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
        pyObject.lib.assertPythonRuns();

        if ((pyObject.lib.getDiagFlags() & PyLib.Diag.F_METH) != 0) {
            System.out.printf("org.jpy.PyProxyHandler: invoke: %s(%s) on pyObject=%s in thread %s\n",
                              method.getName(),
                              Arrays.toString(args),
                              Long.toHexString(this.pyObject.getPointer()),
                              Thread.currentThread());
        }

        return pyObject.lib.callAndReturnValue(this.pyObject.getPointer(),
                                               callableKind == PyLib.CallableKind.METHOD,
                                               method.getName(),
                                               args != null ? args.length : 0,
                                               args,
                                               method.getParameterTypes(),
                                               method.getReturnType());
    }
}
