jpy
===

jpy is a *bi-directional* Java-Python bridge which you can use to embed Java code in Python programs or the other
way round. It has been designed particularly with regard to maximum data transfer speed between the two languages.
It comes with a number of outstanding features:

* Fully translates Java class hierarchies to Python
* Transparently handles Java method overloading
* Support of Java multi-threading
* Fast and memory-efficient support of primitive Java array parameters via Python buffers (http://docs.python.org/3.3/c-api/buffer.html),
  e.g. numpy arrays (http://docs.scipy.org/doc/numpy/reference/arrays.html)
* Support of Java methods that modify primitive Java array parameters (mutable parameters)
* Java arrays translate into Python sequence objects
* Java API for accessing Python objects (`jpy.jar`)

The initial development of jpy has been driven by the need to write Python extensions to an established scientific
imaging application programmed in Java, namely ESA BEAM (http://www.brockmann-consult.de/beam/).
Writing such Python plug-ins for a Java application usually requires a bi-directional communication between Java and
Python, namely the Python extension code must be able to calling back into the Java APIs.


How to Build
------------

After you have checked out [jpy from GitHub](https://github.com/bcdev/jpy) or downloaded the sources, open a terminal
window and cd into the check-out directory. Then you first have to build and install the jpy Python module and
then the jyp Java library.


### Linux / Darwin

You will need
* Python 3.3 or higher (make sure it is the *dev* version)
* JDK 1.7 or higher
* Maven 3
* gcc

To build and test the jpy Python module use the following commands:

    > export JDK_HOME=<path to the JDK installation directory>
    > export JAVA_HOME=$JDK_HOME
    > python3.3 setup.py install --user

On Darwin, you may find the current JDK/Java home using the following expression:

    > export JDK_HOME=$(/usr/libexec/java_home)
    
If you encounter linkage errors during setup saying that something like a ``libjvm.so`` (Linux) or ``libjvm.dylib`` (Darwin) cannot be found, then you can try adding its containing directory to the ``LD_LIBRARY_PATH`` environment variable, e.g.:

    > export LD_LIBRARY_PATH=$JDK_HOME/jre/lib/server:$LD_LIBRARY_PATH


### Microsoft Windows

You will need
* [Python 3.3](http://www.python.org/) or higher
* [JDK 1.7](http://www.oracle.com/technetwork/java/javase/downloads/) or higher
* [Maven 3](http://maven.apache.org/)
* [Microsoft Windows SDK 7.1](http://www.microsoft.com/en-us/download/details.aspx?id=8279)

If you build for a 32-bit Python, make sure to also install a 32-bit JDK. Accordingly, for a 64-bit Python, you will
need a 64-bit JDK.

The Python setup tools (``distutils``) can make use of the command-line C/C++ compilers of the free Microsoft Windows SDK.
These will by used by ``distutils`` if the ``DISTUTILS_USE_SDK`` environment variable is set. The compilers are made accessible via
the command-line by using the ``setenv`` tool of the Windows SDK. In order to install the Windows SDK do the following

1. If you already use Microsoft Visual C++ 2010, make sure to uninstall the x86 and amd64 compiler redistributables first. Otherwise the installation of the Windows SDK will definitely fail. This may also apply to higher versions of Visual C++.
2. Download and install [Windows SDK 7.1](http://www.microsoft.com/en-us/download/details.aspx?id=8279). (This step failed for me the first time. A second 'repair' install was successful.)
3. Download and install [Windows SDK 7.1 SP1](http://www.microsoft.com/en-us/download/details.aspx?id=4422).

Open the command-line and execute

    > "C:\Program Files\Microsoft SDKs\Windows\v7.1\bin\setenv" /x64 /release

to prepare a build of the 64-bit version of jpy. Use

    > "C:\Program Files\Microsoft SDKs\Windows\v7.1\bin\setenv" /x86 /release

to prepare a build of the 32-bit version of jpy. Now set other environment variables.

    > SET DISTUTILS_USE_SDK=1
    > SET JDK_HOME=<path to the JDK installation directory>
    > SET JAVA_HOME=%JDK_HOME%
    > SET PATH=%JDK_HOME%\jre\bin\server;%PATH%

Then, to actually build and test the jpy Python module use the following command:

    > python setup.py install

To create a jpy Windows executable installer, use

    > python setup.py bdist_wininst


How to modify
-------------

The source distribution directory layout is based on Maven common directory structure.

todo - put dir layout here and explain parts


After changing signatures of native methods in `src/main/java/org/jpy/PyLib.java`, you need to compile the Java classes
and regenerate the C headers for the `PyLib` class.

    > mvn compile
    > javah -d src/main/c/jni -v -classpath target/classes org.jpy.PyLib

and adapt changes `org_jpy_PyLib.c` according to newly generated `org_jpy_PyLib.h` and `org_jpy_PyLib_Diag.h`.
Files are found in `src/main/c/jni/`. Then run `setup.py` again as indicated above.



C programming guideline
-----------------------

* Follow style used in Python itself
* Python type global variable names: `J<type>_Type`
* Python type instance structs: `JPy_J<type>`
* Python function decl for a type: `J<type>_<FunctionName>(JNIEnv* jenv, JPy_J<type>* <type>, ...)`
* The pointer is always the first parameter, only type slots obtain their `jenv` from `JPy_GetJEnv()`
* Python slots function for a type: `J<type>_<slot_name>(JNIEnv* jenv, JPy_J<type>* self, ...)`
* Usually functions shall indicate errors by returning NULL or -1 on error.
  Callers can expect that the PyErr_SetError has been set correctly and thus simply
  return NULL or -1 again.
  Exception: very simple functions, e.g. `JObj_Check()`, can go without error status indication.
* Naming conventions:

    * jpy_jtype.h/c - The Java Meta-Type
        * JPy_JType type
        * JType_xxx() functions
    * jpy_jobj.h/c  - The Java Object Wrapper
        * JPy_JObj type
        * JObj_xxx() functions
    * jpy_jmethod.h/c - The Java Method Wrapper
        * JPy_JMethod type
        * JPy_JOverloadedMethod type
        * JMethod_xxx() functions
        * JOverloadedMethod_xxx() functions
    * jpy_jfield.h/c - The Java Field Wrapper
        * JPy_JField type
        * JField_xxx() functions
    * jpy_conv.h/c - Conversion of Python objects from/to Java values
        * JPy_From<JType> functions / JPy_FROM_<JTYPE> macros create Python objects (new references!) from Java types
        * JPy_As<JType> functions / JPy_AS_<JTYPE> macros convert from Python objects to Java types
    * jpy_diag.h/c - Control of outputting diagnostic info
        * JPy_Diag type
        * JPy_DIAG_F_<name> macros
        * JPy_DIAG_PRINT(flags, format, ...) macros
    * jpy_module.h/c - The 'jpy' module definition
        * JPy_xxx() functions
    * jni/org_jpy_PyLib.h - generated by javah from PyLib.java
    * jni/org_jpy_PyLib_Diag.h - generated by javah from PyLib.java
    * jni/org_jpy_PyLib.c - native implementations from PyLib.java
