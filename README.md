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


How to build
------------

> SET JDK_HOME=%JDK32_HOME%
> SET VS90COMNTOOLS=%VS100COMNTOOLS%
> SET PATH=C:\Program Files (x86)\Java\jdk1.7.0_07\jre\bin\server;%PATH%
> org.jpy.python setup.py install

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
Non-final, static class fields are not supported. 
Reason: In jpy, Java classes are represented in Python as (dynamically allocated) built-in 
extension types. Built-in extension types cannot have (as of Python 3.3) static, computed 
attributes which we would need for getting/setting Java static class fields. 
See also
* http://stackoverflow.com/questions/10161609/class-property-using-org.jpy.python-c-api
* http://joyrex.spc.uchicago.edu/bookshelves/org.jpy.python/cookbook/pythoncook-CHP-16-SECT-6.html

Public final static fields are represented as normal (non-computed) type attributes. Their
values are Python representations of the final Java values. The limitation here is, that they 
can be overwritten from Python, because Python does not know final/constant attributes. This could
only be achieved with computed attributes, but as said before, they are not supported for 
built-in extension types. 

Current TODO
------------
* Support Java fields



Design Issues
-------------
* Add a new module function 'import' which calls 'get_class' but then creates new modules up the last dot in the class name.
  E.g. java.io.File will create the module 'java', than create 'io' and add this to 'java' and then add class 'File' to 'io'.
* Add 'get_ref' method to JObj_Type so that we can easily test for Java object identity
* Add 'classpath' parameter to 'create_jvm' function. It is a list of path entries.
  This is needed because using the JVM option '-Djava.class.path=<classpath>' is platform-specific due to the
  platform-dependent path separator. (todo: are there any other platform-specific JVM options?)

* If we only have one JOverloadedMethod then we should use JMethod instead. JOverloadedMethod must be a sub-type of JMethod.
* Add various modes of operation:
  - actual_type (retrieve the actual class of a returned Java object or use the return type of the Java method declaration).
    See JPy_FromJObject() in jpy_jmethod.c

* Make use of PyErr_Format()
* Perform massive JNI error checking. Use this ideom:

jthrowable e = env->ExceptionOccurred();
if (e != NULL) {
	env->ExceptionClear();

	jmethodID toString = env->GetMethodID(env->FindClass("java/lang/Object"), "toString", "()Ljava/lang/String;");
	jstring estring = (jstring) env->CallObjectMethod(e, toString);

	jboolean isCopy;
	message = env->GetStringUTFChars(estring, &isCopy);

    PyErr_Format(format, message)
	...
}

Wrap Java exceptions into our Python JException_Type.


See also
--------

* http://www.jython.org/
* http://jyni.org/
* https://code.google.com/p/jynx/