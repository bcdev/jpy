#!/usr/bin/env python3

__author__ = "Norman Fomferra, Brockmann Consult GmbH"
__copyright__ = "Copyright (C) 2014 Brockmann Consult GmbH"
__license__ = "GPL v3"
__version__ = "0.8-SNAPSHOT"

import sys
import os
import os.path
import sysconfig
import platform
from distutils.core import setup
from distutils.extension import Extension

import src.main.python.jpyutil as jpyutil

IS64 = sys.maxsize > 2 ** 32
ISPY3 = sys.version_info >= (3, 0, 0,)
WIN32 = platform.system() == 'Windows'
LINUX = platform.system() == 'Linux'
DARWIN = platform.system() == 'Darwin'

# e.g. java_home_dir = '/home/marta/jdk1.7.0_15'
jdk_home_dir = os.environ.get('JAVA_HOME', None)
if jdk_home_dir is None:
    jdk_home_dir = os.environ.get('JDK_HOME', None)
    if jdk_home_dir:
        os.environ['JAVA_HOME'] = jdk_home_dir
        print('Warning: Environment variable "JAVA_HOME" not set, using "JDK_HOME" instead')
if jdk_home_dir is None:
    print('Error: Environment variable "JAVA_HOME" must be set to a JDK (>= v1.6) installation directory')
    exit(1)

print('Building a %s-bit library for a %s system with Java at %s' % ('64' if IS64 else '32', platform.system(), jdk_home_dir))

jvm_dll_path = jpyutil.find_jvm_dll_path(jdk_home_dir)
if not jvm_dll_path:
    print('Error: Cannot find any JVM shared library')
    exit(1)

jvm_dll_dir = os.path.dirname(jvm_dll_path)

src_main_c_dir = os.path.join('src', 'main', 'c')
src_test_py_dir = os.path.join('src', 'test', 'python')

sources = [
    os.path.join(src_main_c_dir, 'jpy_module.c'),
    os.path.join(src_main_c_dir, 'jpy_diag.c'),
    os.path.join(src_main_c_dir, 'jpy_conv.c'),
    os.path.join(src_main_c_dir, 'jpy_compat.c'),
    os.path.join(src_main_c_dir, 'jpy_jtype.c'),
    os.path.join(src_main_c_dir, 'jpy_jarray.c'),
    os.path.join(src_main_c_dir, 'jpy_jobj.c'),
    os.path.join(src_main_c_dir, 'jpy_jmethod.c'),
    os.path.join(src_main_c_dir, 'jpy_jfield.c'),
    os.path.join(src_main_c_dir, 'jni/org_jpy_PyLib.c'),
    ]

headers = [
    os.path.join(src_main_c_dir, 'jpy_module.h'),
    os.path.join(src_main_c_dir, 'jpy_diag.h'),
    os.path.join(src_main_c_dir, 'jpy_conv.h'),
    os.path.join(src_main_c_dir, 'jpy_compat.h'),
    os.path.join(src_main_c_dir, 'jpy_jtype.h'),
    os.path.join(src_main_c_dir, 'jpy_jarray.h'),
    os.path.join(src_main_c_dir, 'jpy_jobj.h'),
    os.path.join(src_main_c_dir, 'jpy_jmethod.h'),
    os.path.join(src_main_c_dir, 'jpy_jfield.h'),
    os.path.join(src_main_c_dir, 'jni/org_jpy_PyLib.h'),
    ]

python_tests = [
    os.path.join(src_test_py_dir, 'jpy_array_test.py'),
    os.path.join(src_test_py_dir, 'jpy_field_test.py'),
    os.path.join(src_test_py_dir, 'jpy_retval_test.py'),
    os.path.join(src_test_py_dir, 'jpy_rt_test.py'),
    os.path.join(src_test_py_dir, 'jpy_mt_test.py'),
    os.path.join(src_test_py_dir, 'jpy_exception_test.py'),
    os.path.join(src_test_py_dir, 'jpy_overload_test.py'),
    os.path.join(src_test_py_dir, 'jpy_typeconv_test.py'),
    os.path.join(src_test_py_dir, 'jpy_typeres_test.py'),
    os.path.join(src_test_py_dir, 'jpy_modretparam_test.py'),
    os.path.join(src_test_py_dir, 'jpy_gettype_test.py'),
    os.path.join(src_test_py_dir, 'jpy_diag_test.py'),
    ]

include_dirs = [src_main_c_dir, os.path.join(jdk_home_dir, 'include')]
library_dirs = [jvm_dll_dir]
libraries = ['jvm']
define_macros = []
extra_link_args = []
extra_compile_args = []

if WIN32:
    define_macros += [('WIN32', '1')]
    include_dirs += [os.path.join(jdk_home_dir, 'include', 'win32')]
    library_dirs += [os.path.join(jdk_home_dir, 'lib')]
elif LINUX:
    include_dirs += [os.path.join(jdk_home_dir, 'include', 'linux')]
    libraries += ['python' + sysconfig.get_config_var('VERSION') + (sys.abiflags if ISPY3 else '')]
    extra_link_args += ['-Xlinker', '-rpath', jvm_dll_dir]
elif DARWIN:
    include_dirs += [os.path.join(jdk_home_dir, 'include', 'darwin')]
    libraries =+ ['python' + sysconfig.get_config_var('VERSION') + (sys.abiflags if ISPY3 else '')]
    library_dirs += [os.path.join(sys.exec_prefix, 'lib')]
    extra_link_args += ['-Xlinker', '-rpath', jvm_dll_dir]


with open('README.rst') as file:
    long_description = file.read()

with open('CHANGES.txt') as file:
    changelog = file.read()


setup(name='jpy',
      description='Bi-directional Python-Java bridge',
      long_description=long_description + '\n\n' + changelog,
      version=__version__,
      platforms='Windows, Linux, Darwin',
      author=__author__,
      author_email='norman.fomferra@brockmann-consult.de',
      maintainer='Brockmann Consult GmbH',
      maintainer_email='norman.fomferra@brockmann-consult.de',
      license='GPL 3',
      url='https://github.com/bcdev/jpy',
      download_url='https://pypi.python.org/pypi/jpy/' + __version__,
      package_dir = {'': os.path.join('src', 'main', 'python')},
      py_modules = ['jpyutil'],
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


if 'install' in sys.argv:

    print("Importing module 'jpy' in order to retrieve its shared library location...")

    jpyutil.preload_jvm_dll(jvm_dll_path)
    import jpy

    jpy_lib_path = jpy.__file__
    jpy_exec_path = sys.exec_prefix
    print("Its location is: " + jpy_lib_path)

    user_home = os.path.expanduser('~')
    user_jpy = os.path.join(user_home, '.jpy')
    python_dll_path = jpyutil.find_python_dll_path()

    print('python_dll_path =', python_dll_path)

    from datetime import datetime

    print('Writing this information to file:', user_jpy)
    with open(user_jpy, 'w') as f:
        f.write('# Created by jpy/setup.py on ' + str(datetime.now()) + '\n')
        if python_dll_path:
            f.write('python.lib = ' + python_dll_path.replace('\\', '\\\\') + '\n')
        else:
            f.write('# python.lib = \n')
        f.write('jvm.lib = ' + jvm_dll_path.replace('\\', '\\\\') + '\n')
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


