#!/usr/bin/env python3

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
WIN32 = platform.system() == 'Windows'
LINUX = platform.system() == 'Linux'
DARWIN = platform.system() == 'Darwin'
print('Building a %s-bit library for a %s system' % ('64' if IS64 else '32', platform.system()))

if WIN32 and os.environ.get('VS90COMNTOOLS', None) is None:
    print('Note: If you get an error saying "Unable to find vcvarsall.bat",')
    print('      you may need to set environment variable VS90COMNTOOLS.')
    print('      If you use Visual Studio 2011, then: SET VS90COMNTOOLS=%VS100COMNTOOLS%,')
    print('      if you use Visual Studio 2012, then: SET VS90COMNTOOLS=%VS110COMNTOOLS%.')

sources = [
    'src/main/c/jpy_module.c',
    'src/main/c/jpy_carray.c',
    'src/main/c/jpy_conv.c',
    'src/main/c/jpy_jtype.c',
    'src/main/c/jpy_jmethod.c',
    'src/main/c/jpy_jfield.c',
    'src/main/c/jpy_jobj.c',
    'src/main/c/jni/org_jpy_python_PyLib.c',
]

headers = [
    'src/main/c/jpy_module.h',
    'src/main/c/jpy_carray.h',
    'src/main/c/jpy_conv.h',
    'src/main/c/jpy_jtype.h',
    'src/main/c/jpy_jmethod.h',
    'src/main/c/jpy_jfield.h',
    'src/main/c/jpy_jobj.h',
    'src/main/c/jni/org_jpy_python_PyLib.h',
]

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
    library_dirs = [JDK_HOME + '/jre/lib/i386/server',
                    JDK_HOME + '/lib']
elif LINUX:
    include_dirs += [JDK_HOME + '/include', JDK_HOME + '/include/linux']
    libraries = ['jvm', 'python' + sysconfig.get_config_var('VERSION') + sys.abiflags]
    if IS64:
        library_dirs = [JDK_HOME + '/jre/lib/amd64/server',
                        JDK_HOME + '/lib']
    else:
        library_dirs = [JDK_HOME + '/jre/lib/i386/server',
                        JDK_HOME + '/lib']
elif DARWIN:
    include_dirs += [JDK_HOME + '/include', JDK_HOME + '/include/darwin']
    libraries = ['jvm', 'python' + sysconfig.get_config_var('VERSION') + sys.abiflags]
    library_dirs = [JDK_HOME + '/jre/lib/server',
                    JDK_HOME + '/lib',
                    os.path.join(sys.exec_prefix, 'lib')]

setup(name='jpy',
      description='Python/Java Bridge',
      long_description='A bidirectional Python/Java bridge',
      version='0.4',
      platforms='Python 3.3, Java 1.7',
      author='Norman Fomferra, Brockmann Consult GmbH',
      author_email='beam@brockmann-consult.de',
      maintainer='Brockmann Consult GmbH',
      maintainer_email='beam@brockmann-consult.de',
      license='GPL 3',
      url='http://www.brockmann-consult.de/beam/',
      download_url='http://www.brockmann-consult.de/beam/',
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

    print("Importing module 'jpy' in order get its shared library location...")

    import jpy

    jpy_lib_path = jpy.__file__
    jpy_exec_path = sys.exec_prefix
    print("Its location is: " + jpy_lib_path)

    user_home = os.path.expanduser("~")
    user_jpy = os.path.join(user_home, '.jpy')

    from datetime import datetime

    print('Writing this information to file:', user_jpy)
    with open(user_jpy, 'w', encoding='utf-8') as f:
        f.write('# Created by jpy/setup.py on ' + str(datetime.now()) + '\n')
        f.write('# python.lib = \n')
        f.write('jpy.lib = ' + jpy_lib_path.replace('\\', '\\\\') + '\n')
        f.write('jpy.exec_path = ' + jpy_exec_path.replace('\\', '\\\\') + '\n')

    os.system(sys.executable + ' src/test/python/jpy_field_test.py')
    os.system(sys.executable + ' src/test/python/jpy_retval_test.py')
    os.system(sys.executable + ' src/test/python/jpy_array_test.py')
    os.system(sys.executable + ' src/test/python/jpy_rt_test.py')
    os.system(sys.executable + ' src/test/python/jpy_exception_test.py')
    os.system(sys.executable + ' src/test/python/jpy_overload_test.py')
    os.system(sys.executable + ' src/test/python/jpy_typeconv_test.py')
    os.system(sys.executable + ' src/test/python/jpy_typeres_test.py')

    print('Note: if any of the above tests fails, make sure to compile Java test sources first.')