########
Tutorial
########

*********************
Using jpy with Python
*********************


Getting Started
===============



After installing jpy either directly using the source distribution or the Windows installer the Java Virtual Machine


Frequently seen problems
------------------------

The jpy Python-to-Java bridge requires Python 3.3 or higher and Java 6 or higher. In order to work, jpy
must be able to find the installed Java Virtual Machine (JVM). If the JVM cannot be found, you will get error message
similar to the following ones when importing jpy. On Unix/Darwin::

    >>> import jpy
    bla bla


If the JVM has been found but it does not match the Python interpreter platform::

    >>> import jpy
    Exception in thread "main" java.lang.UnsatisfiedLinkError: C:\Python33-amd64\Lib\site-packages\jpy.pyd: Can't load AMD 64-bit .dll on a IA 32-bit platform


Using the Java Standard Library
===============================


Calling your Java Classes from Python
=====================================


Primitive array parameters that are mutable
-------------------------------------------


Primitive array parameters that are return value
------------------------------------------------



*******************
Using jpy with Java
*******************


Getting Started
===============


Using the Python Standard Library
=================================


Calling your Python functions from Java
=======================================


Extending Java with Python
==========================
