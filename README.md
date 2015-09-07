
jpy Python-Java bridge
======================

jpy is a **bi-directional** Python-Java bridge which you can use to embed Java code in Python programs or the other
way round. It has been designed particularly with regard to maximum data transfer speed between the two languages.
It comes with a number of outstanding features:

* Fully translates Java class hierarchies to Python
* Transparently handles Java method overloading
* Support of Java multi-threading
* Fast and memory-efficient support of primitive Java array parameters via [Python buffers](http://docs.python.org/3.3/c-api/buffer.html)
  (e.g. [numpy arrays](http://docs.scipy.org/doc/numpy/reference/arrays.html)
* Support of Java methods that modify primitive Java array parameters (mutable parameters)
* Java arrays translate into Python sequence objects
* Java API for accessing Python objects (`jpy.jar`)

jpy has been tested with Python 2.7, 3.3, 3.4 and Oracle Java 7 and 8 JDKs. 

The initial development of jpy has been driven by the need to write Python extensions to an established scientific
imaging application programmed in Java, namely the [BEAM](http://www.brockmann-consult.de/beam/) toolbox
funded by the European Space Agency (ESA) and is now continued through
[SNAP](http://step.esa.int/main/toolboxes/snap/), SeNtinel Application Platform project.
Writing such Python plug-ins for a Java application usually requires a bi-directional communication between Python and
Java since the Python extension code must be able to call back into the Java APIs.

For more information please have a look into jpy's

* [documentation](http://jpy.readthedocs.org/en/latest/)
* [source repository](https://github.com/bcdev/jpy)
* [issue tracker](https://github.com/bcdev/jpy/issues?state=open)


How to build on Linux and Mac
-----------------------------

Install a JDK 8, preferrably the Oracle distribution. Set JDK_HOME or JPY_JDK_HOME to point to your JDK installation 
and run the build script: 

    > export JDK_HOME=<your-jdk-dir>
    > python setup.py --maven build

On success, the output is found in the `build` directory.

How to build on Windows
-----------------------

If you are on Windows, please note that if you run a 32-bit Python you'll also need a 32-bit JDK.
Set JDK_HOME or JPY_JDK_HOME to point to your JDK installation. You'll need Windows SDK 7.1 or Visual Studio C++ to 
build the sources. With Windows SDK 7.1

    > SET VS90COMNTOOLS=C:\Program Files (x86)\Microsoft Visual Studio 12.0\Common7\Tools\
    > SET DISTUTILS_USE_SDK=1
    > C:\Program Files\Microsoft SDKs\Windows\v7.1\bin\setenv /x64 /release
    > SET JDK_HOME=<your-jdk-dir>
    > python setup.py --maven build
    
With Visual Studio 14 and higher it is much easier:

    > SET VS100COMNTOOLS=C:\Program Files (x86)\Microsoft Visual Studio 14.0\Common7\Tools\
    > SET JDK_HOME=<your-jdk-dir>
    > python setup.py --maven build

On success, the output is found in the `build` directory.