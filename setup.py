#!/usr/bin/env python3

import sys
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
    'src/main/c/jpy_buffer.c',
    'src/main/c/jpy_jtype.c',
    'src/main/c/jpy_jmethod.c',
    'src/main/c/jpy_jfield.c',
    'src/main/c/jpy_jobj.c',
    'src/main/c/jni/org_jpy_python_PyLib.c',
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

if LINUX:
    include_dirs += [JDK_HOME + '/include', JDK_HOME + '/include/linux']
    libraries = ['jvm']
    if IS64:
        library_dirs = [JDK_HOME + '/jre/lib/amd64/server',
                        JDK_HOME + '/lib']
    else:
        library_dirs = [JDK_HOME + '/jre/lib/i386/server',
                        JDK_HOME + '/lib']

if DARWIN:
    include_dirs += [JDK_HOME + '/include', JDK_HOME + '/include/darwin']
    libraries = ['jvm']
    library_dirs = [JDK_HOME + '/jre/lib/server/',
                    JDK_HOME + '/lib']

setup(name='jpy',
      description='Java Python Bridge',
      long_description='A bridge from Python to Java',
      version='0.1',
      platforms='Python 3, Java 1.7',
      author='Brockmann Consult GmbH',
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
                             define_macros=define_macros
      )]
)

if sys.argv[1] == 'install':
    import shutil

    if WIN32:
        src = os.path.join(sys.exec_prefix, 'Lib', 'site-packages', 'jpy.pyd')
        dst = 'jpy.dll'
    if LINUX:
        src = os.path.join(sys.exec_prefix, 'lib', 'python' + str(sys.version_info.major) + '.' + str(sys.version_info.minor), 'site-packages', 'jpy.so')
        dst = 'libjpy.so'
    if DARWIN:
        src = os.path.join(sys.exec_prefix, 'lib', 'python' + str(sys.version_info.major) + '.' + str(sys.version_info.minor), 'site-packages', 'jpy.so')
        dst = 'libjpy.dylib'

    print('Copying', src, 'to', dst)
    shutil.copyfile(src, dst)
