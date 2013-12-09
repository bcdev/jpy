jpy
===

Why jpy?
--------

Advantages over the 'beampy' codegen approach:
* Can load *any* Java classes, not only ones for which code has been generated
* Transparently deals with Java method overloads, instead of renaming methods
* It is more pythonic: Real constructors instead of generated static New_<Type> methods
* Java Class inheritance is be preserved
* Source code is easier to understand and maintain
* Different modes of operations can be used without static compilation (e.g. use buffers)
* Since the binding is dynamic, it can be used with any BEAM version instead of generating code
  for a given BEAM source code version
* Can use Python to configure the JVM (e.g. class path assembly)

Disadvantages to the 'beampy' codegen approach:
* No in-built documentation for classes, fields and methods (but actually Java docs can be used instead)
* Slower, method resolution at runtime, memory overhead



Other
-----

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