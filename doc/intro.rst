
############
Introduction
############

jpy is a bi-directional Java-Python bridge which you can use to embed Java code in Python programs or the other
way round. It has been designed particularly with regard to maximum data transfer speed between the two languages.
It comes with a number of outstanding features:

* Fully translates Java class hierarchies to Python
* Transparently handles Java method overloading
* Support of Java multi-threading
* Fast and memory-efficient support of primitive Java array parameters via `Python buffers <http://docs.python.org/3.3/c-api/buffer.html>`_
  (e.g. `numpy arrays <http://docs.scipy.org/doc/numpy/reference/arrays.html>`_)
* Support of Java methods that modify primitive Java array parameters (mutable parameters)
* Java arrays translate into Python sequence objects
* Java API for accessing Python objects (``jpy.jar``)

The initial development of jpy has been driven by the need to write Python extensions to an established scientific
imaging application programmed in Java (`ESA BEAM <http://www.brockmann-consult.de/cms/web/beam/>`_).
Writing such Python plug-ins for a Java application usually requires a bi-directional communication between Java and
Python, namely the Python extension code must be able to calling back into the Java APIs.


************
How It Works
************


Calling Java from Python
========================

Instantiate Python objects from Java classes and call their public methods and fields::

    import jpy

    File = jpy.get_class('java.io.File')

    file = File('test/it')
    name = file.getName()



Calling Python from Java
========================

Call Python functions from Java::

    PyModule sys = PyModule.importModule("sys")
    PyObject path = sys.getAttribute("path")
    String value = path.getStringValue();


Instantiating Java objects from Python modules or classes and call Python module functions or Python class methods
is as simple. Assuming we have a Java interface ``PlugIn.java`` ::

    public interface PlugIn {
        String[] process(String arg);
    }

and a Python implementation ``bibo_plugin.py`` ::

    class BiboPlugIn:
        def process(self, arg):
            return arg.split();


then we can call the Python code from Java as follows ::

    PyModule plugInModule = PyLib.importModule("bibo_plugin")
    PyObject plugInClass = plugInModule.getAttribute("BiboPlugIn")
    PyObject plugInObj = plugInClass.call();
    PlugIn plugIn = plugInObj.cast(PlugIn.class);

    String[] result = plugIn.process('Abcdefghi jkl mnopqr stuv wxy z')


**************
Technical Info
**************


The jpy Python module is entirely written in the C programming language. The same resulting shared library is used
as a Python jpy module and also as native library for the Java module (``jpy.jar``).

