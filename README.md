jpy
===

Features
--------

* 1:1 translation from public Java to Python classes
* Bidirectional communication:
  - embed Java VM in Python programs
  - embed Python VM in Java programs
* Preserves Java type hierarchy
* Transparently handles Java method overloading 
* Fast & efficient support of primitive Java array parameters via Python buffers.
  E.g. use a numpy array argument, whenever a Java primitive array parameter is expected
  Also supports Java methods that modify primitive Java array passed in as buffers 
  (mutable arrays)

System Requirements
-------------------
* Python 3.3
* Python 3.3 Dev (Unix/Darwin)
* JDK 1.7 (at build time)
* JRE 1.7 (at runtime)
* Microsoft Visual Studio 10 (Windows)

How to build on Windows
-----------------------

Currently, only 32-bit versions are supported.

    > SET JDK_HOME=%JDK32_HOME%
    > SET VS90COMNTOOLS=%VS100COMNTOOLS%
    > SET PATH=%JDK_HOME%\jre\bin\server;%PATH%
    > python setup.py install

How to build on Unix/Darwin
---------------------------

    > export JDK_HOME=%JDK32_HOME%
    > export path=$JDK_HOME\jre\bin\server;$path
    > python setup.py install

How to modify
-------------

After changing org.jpy.python.PyInterpreter, run

    > javah -d src/main/c/jni -v -classpath target/classes org.jpy.python.PyLib

and adapt changes src/main/c/jni/org_jpy_PythonInterpreter.c according to newly generated
src/main/c/jni /src/main/c/jni/org_jpy_PythonInterpreter.h



Why jpy?
--------

Advantages over the 'beampy' codegen approach:
* Can load *any* Java classes, not only ones for which code has been generated
* Transparently deals with Java method overloads, instead of renaming methods
* It is more pythonic: Real constructors instead of generated static New_<Type> methods
* Java Class inheritance is be preserved
* Source code is easier to understand and maintain as with the codegen approach
* Different modes of operations can be used without static compilation (e.g. use buffers)
* Since the binding is dynamic, it can be used with any BEAM version instead of generating code
  for a given BEAM source code version
* Can use Python to configure the JVM (e.g. class path assembly)

Disadvantages to the 'beampy' codegen approach:
* No in-built documentation for classes, fields and methods (but actually Java docs can be used instead)
* Slower, method resolution at runtime, memory overhead


Current limitations
-------------------
*Non-final, static class fields are not supported.*
Reason: In jpy, Java classes are represented in Python as (dynamically allocated) built-in 
extension types. Built-in extension types cannot have (as of Python 3.3) static, computed 
attributes which we would need for getting/setting Java static class fields. 
See also
* http://stackoverflow.com/questions/10161609/class-property-using-org.jpy.python-c-api
* http://joyrex.spc.uchicago.edu/bookshelves/org.jpy.python/cookbook/pythoncook-CHP-16-SECT-6.html

*Public final static fields are represented as normal (non-computed) type attributes.*
Their values are Python representations of the final Java values. The limitation here is, that they
can be overwritten from Python, because Python does not know final/constant attributes. This could
only be achieved with computed attributes, but as said before, they are not supported for 
built-in extension types.

*Java generics are not supported.*
Simply because this hasn't been implemented yet.


Current TODOs
-------------
* Add unit tests to target 95% code coverage.
* Add the possibility to let users pythonically import Java classes: e.g.
     `from java.io import File`
  instead of
     `File = jpy.get_class('java.io.File')`
  This is also how it is done in Jython.
* Instead of getting the jenv pointer from the global function JPy_GetJNIEnv(), add a JNIEnv* as first parameter to
  all functions that require it. Only entry points from Python calls into C shall use the global retrieval function.
* Make it fully multi-threading aware (use global switch 'multi_threaded').
* Assert rigorously that Python reference counting is correct.
* Assert rigorously that Java global/local references are correctly created and released.
* Perform rigorous Python C API error checking.
* Perform rigorous JNI error checking. Wrap Java exceptions into our Python JException_Type.
  Use this ideom (but not for no-memory errors!):

    jthrowable e = (*jenv)->ExceptionOccurred(jenv);
    if (e != NULL) {
        jstring message;
        const char* messageChars;
        jenv->ExceptionClear();
        message = (jstring) (*jenv)->CallObjectMethod(jenv, e, JPy_Object_ToString_MID);
        messageChars = (*jenv)->GetStringUTFChars(jenv, message, NULL);
        PyErr_Format(JException_Type, format, messageChars)
        (*jenv)->ReleaseStringUTFChars(jenv, message, messageChars);
    }


Design Issues
-------------
* Add a new module function 'import' which calls 'get_class' but then creates new modules up the last dot in the class name.
  E.g. java.io.File will create the module 'java', than create 'io' and add this to 'java' and then add class 'File' to 'io'.
* Add 'get_ref' method to JObj_Type so that we can easily test for Java object identity
* If we only have one JOverloadedMethod then we should use JMethod instead. JOverloadedMethod must be a sub-type of JMethod.
* Add various modes of operation:
  - actual_type (retrieve the actual class of a returned Java object or use the return type of the Java method declaration).
    See JPy_FromJObject() in jpy_jmethod.c



See also
--------

* http://www.jython.org/
* http://jyni.org/
* https://code.google.com/p/jynx/