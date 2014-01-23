jpy
===

jpy is a *bi-directional* Java-Python bridge which you can use to embed Java code in Python programs or the other
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


How to Build
------------

After you have checked out jpy from GitHub or downloaded the sources, open a terminal window and cd into the check-out
directory. Then you first have to build and install the jpy Python module and then the jyp Java library.

Windows
^^^^^^^

You will need
* Python 3.3
* JDK 1.7
* Maven 3
* Microsoft Visual C++ 2010 (MSVC)

The Python setup tools make use of the command-line C/C++ compiler of MSVC so you need to set all required compiler
environment variables as described in http://msdn.microsoft.com/en-us/library/f2ccy3wt.aspx. Note that if you use
Microsoft Visual C++ 2010 Express, then only 32-bit versions of jpy can be built. In this case make sure that you also
use the 32-bit versions of Python and the JDK.

To build and test the jpy Python module use the following commands:

    > SET VS90COMNTOOLS=%VS100COMNTOOLS%
    > SET PATH=%JDK_HOME%\jre\bin\server;%PATH%
    > python setup.py install

To create a jpy Windows executable installer, use

    > python setup.py bdist_wininst

To build and test the jpy Java library we use Maven:

    > SET JAVA_HOME=%JDK_HOME%
    > mvn install


Unix/Darwin
^^^^^^^^^^^

You will need
* Python 3.3 Dev (!)
* JDK 1.7
* Maven 3
* gcc

To build and test the jpy Python module use the following commands:

    > export JDK_HOME=%JDK32_HOME%
    > export path=$JDK_HOME\jre\bin\server;$path
    > python3.3 setup.py install --user

To build and test the jpy Java library we use Maven:

    > SET JAVA_HOME=%JDK_HOME%
    > mvn install

How to modify
-------------

The source distribution directory layout is based on Maven.

todo - put dir layout here and explain parts



After changing `org.jpy.PyLib`, run

    > javah -d src/main/c/jni -v -classpath target/classes org.jpy.PyLib

and adapt changes `org_jpy_PyLib.c` according to newly generated `org_jpy_PyLib.h` and `org_jpy_PyLib_Diag.h`.
Files are found in `src/main/c/jni/`.


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


