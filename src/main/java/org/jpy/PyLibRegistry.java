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

import java.util.ArrayList;
import java.util.List;


/**
 * A registry for Python interpreters.
 *
 * @author Norman Fomferra
 * @since 0.9
 */
public class PyLibRegistry {

    private final List<PyLib> libs;

    public PyLibRegistry() {
        this.libs = new ArrayList<>();
    }

    public void add(PyLib lib) {
        this.libs.add(lib);
    }

    public PyLib[] getAll() {
        return this.libs.toArray(new PyLib[0]);
    }
}


