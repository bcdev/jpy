*************
jpy Changelog
*************

Version 0.9 (dev)
=================

This version includes a number of contributions from supportive GitHub users. Thanks to all of you! 

Fixes
-----

* Fixed error caused by missing `sys.argv` in Python when called from Java (issue #81). Fix by Dave Voutila.
* Fixed problem where calling jpy.get_type() too many times causes a memory access error (issue #74). Fix by Dave Voutila.
* Fixed a corruption when retrieving long values (#72). Fix by chipkent. 
* Fixed fatal error when stopping python session (issue #70, #77). Fix by Dave Voutila.

Improvements
------------

* Java `PyObject` is now serializable. Contribution by Mario Briggs. 


Version 0.8
===========

Fixes
-----

* Java interface types don't include methods of extended interfaces (issue #64)
* Loading of jpy DLL fails for user-specific Python installations on Windows (issue #58)
* Java interface types didn't expose java.lang.Object methods (issue #57)
* Java 1-arg static method was confused with a zero-arg non-static method (issue #54)
* Python interpreter crash occurred when executing del statement on Java arrays (issue #52)
* Python extensions loaded from Java couldn't see Python symbols (Linux) (issue #38)

Improvements
------------

* It is now possible to use jpy Java API to work with multiple Python installations (issue #35).
  A tool called 'jpyutil.py' can be used to write configuration files that determine the required shared libraries
  for a given Python versions.
  A new Java system property 'jpy.config' is used to point to a desired configuration file.
* Simplified jpy installation (issue #15):
  - removed need to add JVM path to PATH (Windows) / LD_LIBRARY_PATH (Unix) environment variable
  - removed need to compile Java module using Maven
  - removed need to specify JDK_HOME environment variable, if JAVA_HOME already points to a JDK
 * Added 'jclass' attribute to Python type that wraps a Java class (issue #63) .
 * Java API extensions
  - new jpy.org.PyObject.executeCode() methods
  - new jpy.org.PyModule.getBuiltins() method
  - new jpy.org.PyModule.getMain() method
  - new jpy.org.PyModule.extendSysPath() method
* Java API configuration changes:
  - System property jpy.jpyLib:
  - System property jpy.jdlLib:
  - System property jpy.pythonLib:
  - System property jpy.config:
  - Loaded from
    # File ./jpyconfig.properties
    # Resource /jpyconfig.properties
    # File ${jpy.config}
* Python API configuration changes:
  - Loaded from
    # File ./jpyconfig.py
    # Resource ${jpy-module}/jpyconfig.py
  - Attribute java_home
  - Attribute jvm_dll
* Python API extensions
  - new jpyutil module
    # jpyutil.init_jvm(...)
    # jpyutil.preload_jvm_lib(...)
  - new jpyutil tool
    # usage: jpyutil.py [-h] [--out OUT] [--java_home JAVA_HOME] [--jvm_dll JVM_DLL]
* Added basic support for Java Scripting Engine API (issue #53)

Other changes
-------------
* Switched to Apache 2.0 license from version 0.8 and later (issue #60)


Version 0.7.5
=============

* Fixed bad pointer in C-code which caused unpredictable crashes (issue #43)


Version 0.7.4
=============

* Fixed a problem where jpy crashes with unicode arguments (issue #42)
* Fixed segmentation fault occurring occasionally during installation of jpy (issue #40)
* Improved Java exception messages on Python errors (issue #39)


Version 0.7.3
=============

* Fixed problem where a Java primitive array argument has occasionally not been initialised by a
  related Python buffer argument (issue #37)


Version 0.7.2
=============

* Added backward compatibility with Python 2.7 (issue #34).
* Added Java parameter annotation 'output' (issue #36).
  This is used to optimise passing Python buffer arguments where Java primitive arrays are expected.
* Removed debugging prints of the form "JNI_OnLoad: ..."
* Corrected documentation of jpy.array(type, init) function, which was said to be jpy.array(type, length)
* Removed console dumps that occurred when calling from Java proxies into Python
* Updated Java API documentation and added it to Sphinx doc folder (doc/_static/java-apidoc)
* Added new diagnostic F_ERR flag to Java class PyLib.Diag
* Java class PyLib is no longer instantiable


Version 0.7.1
=============

* Updated README and added MANIFEST.in after recognising that the jpy-0.7.zip distribution misses most of the
  required source files and learning what to do on this case.


Version 0.7
===========

* Initial version.

