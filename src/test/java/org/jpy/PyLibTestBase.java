package org.jpy;

import org.junit.After;
import org.junit.Before;

import java.io.File;

import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;

class PyLibTestBase {

    PyLib lib;

    @Before
    public void setUp() throws Exception {
        String configPath = System.getProperty(PyLibConfig.Props.JPY_CONFIG_KEY);
        if (configPath == null) {
            throw new IllegalStateException(String.format("please set -D%s=<path>, where <path> points to a valid %s file",
                                                          PyLibConfig.Props.JPY_CONFIG_KEY, PyLibConfig.Props.JPY_CONFIG_RESOURCE));
        }
        PyLibConfig config = PyLibConfig.Props.load(new File(configPath));

        lib = new PyLib(config);
        startInterpreter();
    }

    @After
    public void tearDown() throws Exception {
        stopInterpreter();
    }

    protected void startInterpreter() throws Exception {
        lib.startPython(getExtraPaths());
        assertTrue(lib.isPythonRunning());
        lib.setDiagFlags(getDiagFlags());
    }


    protected void stopInterpreter() throws Exception {
        lib.stopPython();
        assertFalse(lib.isPythonRunning());
        lib = null;

    }

    protected int getDiagFlags() {
        return org.jpy.PyLib.Diag.F_OFF;
    }

    protected String[] getExtraPaths() throws Exception{
        return new String[0];
    }
}
