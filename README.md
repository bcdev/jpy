jpy
===

jpy is a *bi-directional* Python-Java bridge which you can use to embed Java code in Python programs or the other
way round. It has been designed particularly with regard to maximum data transfer speed between the two languages.
It comes with a number of outstanding features:

* Fully translates Java class hierarchies to Python
* Transparently handles Java method overloading
* Support of Java multi-threading
* Fast and memory-efficient support of primitive Java array parameters via [Python buffers](http://docs.python.org/3.3/c-api/buffer.html),
  e.g. [numpy arrays](http://docs.scipy.org/doc/numpy/reference/arrays.html)
* Support of Java methods that modify primitive Java array parameters (mutable parameters)
* Java arrays translate into Python sequence objects
* Java API for accessing Python objects (`jpy.jar`)

The initial development of jpy has been driven by the need to write Python extensions to an established scientific
imaging application that is entirely programmed in Java, namely [BEAM](http://www.brockmann-consult.de/beam/).
BEAM is an Earth observation data toolbox and development platform funded by the European Space Agency (ESA).
Writing such Python plug-ins for a Java application usually requires a bi-directional communication between Java and
Python, thus the Python extension code must be able to calling back into the Java APIs.

For more information please have a look into the [jpy documentation](http://jpy.readthedocs.org/en/latest/).




