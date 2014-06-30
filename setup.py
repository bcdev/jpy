#!/usr/bin/env python3

__author__ = "Norman Fomferra, Brockmann Consult GmbH"
__copyright__ = "Copyright (C) 2014 Brockmann Consult GmbH"
__license__ = "GPL v3"
__version__ = "0.7.3"

sources = [
    'src/main/c/jpy_module.c',
    'src/main/c/jpy_diag.c',
    'src/main/c/jpy_conv.c',
    'src/main/c/jpy_compat.c',
    'src/main/c/jpy_jtype.c',
    'src/main/c/jpy_jarray.c',
    'src/main/c/jpy_jobj.c',
    'src/main/c/jpy_jmethod.c',
    'src/main/c/jpy_jfield.c',
    'src/main/c/jni/org_jpy_PyLib.c',
]

headers = [
    'src/main/c/jpy_module.h',
    'src/main/c/jpy_diag.h',
    'src/main/c/jpy_conv.h',
    'src/main/c/jpy_compat.h',
    'src/main/c/jpy_jtype.h',
    'src/main/c/jpy_jarray.h',
    'src/main/c/jpy_jobj.h',
    'src/main/c/jpy_jmethod.h',
    'src/main/c/jpy_jfield.h',
    'src/main/c/jni/org_jpy_PyLib.h',
]

python_tests = [
    'src/test/python/jpy_array_test.py',
    'src/test/python/jpy_field_test.py',
    'src/test/python/jpy_retval_test.py',
    'src/test/python/jpy_rt_test.py',
    'src/test/python/jpy_mt_test.py',
    'src/test/python/jpy_exception_test.py',
    'src/test/python/jpy_overload_test.py',
    'src/test/python/jpy_typeconv_test.py',
    'src/test/python/jpy_typeres_test.py',
    'src/test/python/jpy_modretparam_test.py',
    'src/test/python/jpy_gettype_test.py',
    'src/test/python/jpy_diag_test.py',
]


import sys
import sysconfig
import os
import platform
from distutils.core import setup
from distutils.extension import Extension


# e.g. JDK_HOME = '/home/marta/jdk1.7.0_15'
JDK_HOME = os.environ.get('JDK_HOME', None)
if JDK_HOME is None:
    print('Error: Environment variable "JDK_HOME" must be set to a JDK (>= v1.6) installation directory')
    exit(1)

IS64 = sys.maxsize > 2 ** 32
ISPY3 = sys.version_info >= (3, 0, 0,)
WIN32 = platform.system() == 'Windows'
LINUX = platform.system() == 'Linux'
DARWIN = platform.system() == 'Darwin'
print('Building a %s-bit library for a %s system' % ('64' if IS64 else '32', platform.system()))

include_dirs = ['src/main/c']
library_dirs = []
libraries = []
define_macros = []
extra_link_args = []
extra_compile_args = []

if WIN32:
    define_macros += [('WIN32', '1')]
    include_dirs += [JDK_HOME + '/include', JDK_HOME + '/include/win32']
    libraries = ['jvm']
    library_dirs = [JDK_HOME + '/jre/lib/server',
                    JDK_HOME + '/jre/lib/client',
                    JDK_HOME + '/lib']
elif LINUX:
    include_dirs += [JDK_HOME + '/include', JDK_HOME + '/include/linux']
    libraries = ['jvm', 'python' + sysconfig.get_config_var('VERSION') + (sys.abiflags if ISPY3 else '')]
    library_dirs = [JDK_HOME + '/jre/lib/' + ('amd64' if IS64 else 'i386') + '/server',
                    JDK_HOME + '/jre/lib/' + ('amd64' if IS64 else 'i386') + '/client',
                    JDK_HOME + '/lib']
elif DARWIN:
    include_dirs += [JDK_HOME + '/include', JDK_HOME + '/include/darwin']
    libraries = ['jvm', 'python' + sysconfig.get_config_var('VERSION') + (sys.abiflags if ISPY3 else '')]
    library_dirs = [JDK_HOME + '/jre/lib/server',
                    JDK_HOME + '/jre/lib/client',
                    JDK_HOME + '/lib',
                    os.path.join(sys.exec_prefix, 'lib')]


with open('README.rst') as file:
    long_description = file.read()


setup(name='jpy',
      description='Bi-directional Python-Java bridge',
      long_description=long_description,
      version=__version__,
      platforms='Python ' + ('3.3' if ISPY3 else '2.7') + ', Java 1.7',
      author=__author__,
      author_email='norman.fomferra@brockmann-consult.de',
      maintainer='Brockmann Consult GmbH',
      maintainer_email='norman.fomferra@brockmann-consult.de',
      license='GPL 3',
      url='https://github.com/bcdev/jpy',
      download_url='https://pypi.python.org/pypi/jpy/' + __version__,
      ext_modules=[Extension('jpy',
                             sources,
                             include_dirs=include_dirs,
                             library_dirs=library_dirs,
                             libraries=libraries,
                             extra_link_args=extra_link_args,
                             extra_compile_args=extra_compile_args,
                             define_macros=define_macros,
                             depends=headers
      )]
)


if sys.argv[1] == 'install':

    print("Importing module 'jpy' in order to retrieve its shared library location...")

    import jpy

    jpy_lib_path = jpy.__file__
    jpy_exec_path = sys.exec_prefix
    print("Its location is: " + jpy_lib_path)

    user_home = os.path.expanduser("~")
    user_jpy = os.path.join(user_home, '.jpy')

    from datetime import datetime

    print('Writing this information to file:', user_jpy)
    with open(user_jpy, 'w') as f:
        f.write('# Created by jpy/setup.py on ' + str(datetime.now()) + '\n')
        f.write('# python.lib = \n')
        f.write('jpy.lib = ' + jpy_lib_path.replace('\\', '\\\\') + '\n')
        f.write('jpy.exec_path = ' + jpy_exec_path.replace('\\', '\\\\') + '\n')


    print("Compiling Java code...")
    os.system('mvn clean test-compile')

    print("Executing Python unit tests...")
    result = 0
    for test in python_tests:
        r = os.system(sys.executable + ' ' + test)
        if r != 0:
            result = 1

    if result == 0:
        print("Installing compiled Java code...")
        os.system('mvn install')


