
############
Installation
############


jpy's installation is currently the full build process from sources.
We will try to ease the installation process in the future.

After successful installation you will be able

* to use Java from Python by importing the jpy module ``import jpy`` and
* to use Python from Java by importing the jpy Java API classes ``import org.jpy.*;`` from ``jpy.jar`` on your Java classpath.

You will also find a file called ``.jpy`` in your home folder. This is used by jpy's Java API ``jpy.jar`` in order to
locate the Python interpreter as well as the shared library representing the jpy Python module.

*******************
Getting the Sources
*******************

The first step is to clone or download the jpy repository from the `jpy Project page <https://github.com/bcdev/jpy>`_.
If you like to clone (recommended), use the git tool::

    git clone https://github.com/bcdev/jpy.git

Now change into the checkout directory (``cd jpy``) and follow the setup steps below.

************************
Setup for Linux / Darwin
************************

You will need

* Python 3.3 or higher (make sure it is the *dev* version)
* JDK 1.7 or higher
* Maven 3
* gcc

To build and test the jpy Python module use the following commands::

    export JDK_HOME=<path to the JDK installation directory>
    export JAVA_HOME=$JDK_HOME
    python3.3 setup.py install --user

On Darwin, you may find the current JDK/Java home using the following expression::

    export JDK_HOME=$(/usr/libexec/java_home)

If you encounter linkage errors during setup saying that something like a ``libjvm.so`` (Linux) or ``libjvm.dylib`` (Darwin) cannot be found, then you can try adding its containing directory to the ``LD_LIBRARY_PATH`` environment variable, e.g.::

    export LD_LIBRARY_PATH=$JDK_HOME/jre/lib/server:$LD_LIBRARY_PATH


***************************
Setup for Microsoft Windows
***************************

You will need

* `Python 3.3 <http://www.python.org/>`_ or higher
* `Oracle JDK 1.7 <http://www.oracle.com/technetwork/java/javase/downloads/>`_ or higher
* `Maven 3 <http://maven.apache.org/>`_ or higher
* `Microsoft Windows SDK 7.1 <http://www.microsoft.com/en-us/download/details.aspx?id=8279>`_ or higher

If you build for a 32-bit Python, make sure to also install a 32-bit JDK. Accordingly, for a 64-bit Python, you will
need a 64-bit JDK.

The Python setup tools (``distutils``) can make use of the command-line C/C++ compilers of the free Microsoft Windows SDK.
These will by used by ``distutils`` if the ``DISTUTILS_USE_SDK`` environment variable is set. The compilers are made accessible via
the command-line by using the ``setenv`` tool of the Windows SDK. In order to install the Windows SDK do the following

1. If you already use Microsoft Visual C++ 2010, make sure to uninstall the x86 and amd64 compiler redistributables first. Otherwise the installation of the Windows SDK will definitely fail. This may also apply to higher versions of Visual C++.
2. Download and install `Windows SDK 7.1 <http://www.microsoft.com/en-us/download/details.aspx?id=8279>`_. (This step failed for me the first time. A second 'repair' install was successful.)
3. Download and install `Windows SDK 7.1 SP1 <http://www.microsoft.com/en-us/download/details.aspx?id=4422>`_.

Open the command-line and execute::

    "C:\Program Files\Microsoft SDKs\Windows\v7.1\bin\setenv" /x64 /release

to prepare a build of the 64-bit version of jpy. Use::

    "C:\Program Files\Microsoft SDKs\Windows\v7.1\bin\setenv" /x86 /release

to prepare a build of the 32-bit version of jpy. Now set other environment variables::

    SET DISTUTILS_USE_SDK=1
    SET JDK_HOME=<path to the JDK installation directory>
    SET JAVA_HOME=%JDK_HOME%
    SET PATH=%JDK_HOME%\jre\bin\server;%PATH%

Then, to actually build and test the jpy Python module use the following command::

    python setup.py install

To create a jpy Windows executable installer, use::

    python setup.py bdist_wininst



****************
Typical Problems
****************

When used from Python, jpy must be able to find an installed Java Virtual Machine (JVM) on your computer. This is
usually the one that has been linked to the Python module during the build process.

If the JVM cannot be found, you will have to adapt the ``LD_LIBRARY_PATH`` (Unix) or ``PATH`` (Windows) environment
variables to contain the path to the JVM shared libraries. That is ``libjvm.dylib`` (Darwin), ``libjvm.so`` (Linux) and
``jvm.dll`` (Windows). Make sure to use matching platform architectures, e.g. only use a 64-bit JVM for a 64-bit Python.

Otherwise the JVM may be found but you will get error similar to the following one (Windows in this case)::

    >>> import jpy
    Exception in thread "main" java.lang.UnsatisfiedLinkError: C:\Python33-amd64\Lib\site-packages\jpy.pyd: Can't load AMD 64-bit .dll on a IA 32-bit platform


