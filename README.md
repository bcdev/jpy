[![Build Status](https://travis-ci.org/bcdev/jpy.svg?branch=master)](https://travis-ci.org/bcdev/jpy)
[![Build Status](https://ci.appveyor.com/api/projects/status/ywkcey4nlt0avasf?svg=true)](https://ci.appveyor.com/project/bcdev/jpy)
[![Documentation Status](https://readthedocs.org/projects/jpy/badge/?version=latest)](http://jpy.readthedocs.org/en/latest/?badge=latest)

jpy - a Python-Java Bridge
==========================

jpy is a **bi-directional** Python-Java bridge which you can use to embed Java code in Python programs or the other
way round. It has been designed particularly with regard to maximum data transfer speed between the two languages.
It comes with a number of outstanding features:

* Fully translates Java class hierarchies to Python
* Transparently handles Java method overloading
* Support of Java multi-threading
* Fast and memory-efficient support of primitive Java array parameters via
  [Python buffers](http://docs.python.org/3.3/c-api/buffer.html)
  (e.g. [Numpy arrays](http://docs.scipy.org/doc/numpy/reference/arrays.html))
* Support of Java methods that modify primitive Java array parameters (mutable parameters)
* Java arrays translate into Python sequence objects
* Java API for accessing Python objects (`jpy.jar`)

jpy has been tested with Python 3.8 and OpenJDK 8 on 64-bit Ubuntu Linux, Windows, and macOS.

The initial development of jpy has been driven by the need to write Python extensions to an established scientific
imaging application programmed in Java, namely the [SNAP](http://step.esa.int/) toolbox, the SeNtinel Application 
Platform project, funded by the [European Space Agency](http://www.esa.int/ESA) (ESA).
Writing such Python plug-ins for a Java application usually requires a bi-directional communication between Python and
Java since the Python extension code must be able to call back into the Java APIs.

For more information please have a look into jpy's

* [documentation](http://jpy.readthedocs.org/en/latest/)
* [source repository](https://github.com/bcdev/jpy)
* [issue tracker](https://github.com/bcdev/jpy/issues?state=open)

How to build wheels for Linux and Mac
-------------------------------------

Install a JDK 8, preferably the Oracle distribution. Set JDK_HOME or JPY_JDK_HOME to point to your JDK installation 
and run the build script:

    $ export JDK_HOME=<your-jdk-dir>
    $ export JAVA_HOME=$JDK_HOME
    $ python get-pip.py
    $ python setup.py build maven bdist_wheel

On success, the wheel is found in the `dist` directory.

To deploy the `jpy.jar` (if you don't know why you need this step, this is not for you): ::

    $ mvn clean deploy -DskipTests=true

How to build a wheel for Windows
--------------------------------

Set JDK_HOME or JPY_JDK_HOME to point to your JDK installation. You'll need Windows SDK 7.1 or Visual Studio C++ to 
build the sources. With Windows SDK 7.1::

    > SET VS90COMNTOOLS=C:\Program Files (x86)\Microsoft Visual Studio 12.0\Common7\Tools\
    > SET DISTUTILS_USE_SDK=1
    > C:\Program Files\Microsoft SDKs\Windows\v7.1\bin\setenv /x64 /release
    > SET JDK_HOME=<your-jdk-dir>
    > python get-pip.py
    > python setup.py build maven bdist_wheel
    
With Visual Studio 14 and higher it is much easier::

    > SET VS100COMNTOOLS=C:\Program Files (x86)\Microsoft Visual Studio 14.0\Common7\Tools\
    > SET JDK_HOME=<your-jdk-dir>
    > python get-pip.py
    > python setup.py build maven bdist_wheel

On success, the wheel can be found in the `dist` directory.

How to install from sources
---------------------------
TBD

Releasing jpy
-------------

The target reader of this section is a jpy developer wishing to release a new jpy version.
Note: You need to have Sphinx installed to update the documentation.

1. Make sure all Java *and* Python units tests run green
2. Remove the `-SNAPSHOT` qualifier from versions names in both the Maven `pom.xml` and `setup.py` files.
3. Generate Java API doc by running `mvn javadoc:javadoc` which will update directory `doc/_static`
4. Update documentation, `cd doc` and run `make html` 
5. http://peterdowns.com/posts/first-time-with-pypi.html
