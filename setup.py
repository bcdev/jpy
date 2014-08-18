#!/usr/bin/env python3

__author__ = "Norman Fomferra, Brockmann Consult GmbH"
__copyright__ = "Copyright (C) 2014 Brockmann Consult GmbH"
__license__ = "GPL v3"
__version__ = "0.8-SNAPSHOT"

import sys
import os
import os.path
import platform
import datetime
from distutils import log
from distutils.core import setup
from distutils.extension import Extension
import src.main.python.jpyutil as jpyutil

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

# e.g. java_home_dir = '/home/marta/jdk1.7.0_15'
jdk_home_dir = os.environ.get('JAVA_HOME', None)
if jdk_home_dir is None:
    jdk_home_dir = os.environ.get('JDK_HOME', None)
    if jdk_home_dir:
        os.environ['JAVA_HOME'] = jdk_home_dir
        log.warn('warning: environment variable "JAVA_HOME" not set, using "JDK_HOME" instead')
if jdk_home_dir is None:
    log.error('error: environment variable "JAVA_HOME" must be set to a JDK (>= v1.6) installation directory')
    exit(1)

log.info(
    'Building a %s-bit library for a %s system with Java at %s' % (
        '64' if jpyutil.IS64BIT else '32', platform.system(), jdk_home_dir))

jvm_dll_path = jpyutil.find_jvm_dll_path(jdk_home_dir)
if not jvm_dll_path:
    log.error('error: Cannot find any JVM shared library')
    exit(1)

jvm_dll_dir = os.path.dirname(jvm_dll_path)

include_dirs = [src_main_c_dir, os.path.join(jdk_home_dir, 'include')]
library_dirs = [jvm_dll_dir]
#libraries = [jpyutil.JVM_LIB_NAME, jpyutil.PYTHON_LIB_NAME]
libraries = [jpyutil.JVM_LIB_NAME]
define_macros = []
extra_link_args = []
extra_compile_args = []

if platform.system() == 'Windows':
    define_macros += [('WIN32', '1')]
    include_dirs += [os.path.join(jdk_home_dir, 'include', 'win32')]
    library_dirs += [os.path.join(jdk_home_dir, 'lib')]
elif platform.system() == 'Linux':
    include_dirs += [os.path.join(jdk_home_dir, 'include', 'linux')]
    libraries += ['dl']
    extra_link_args += ['-Xlinker', '-rpath', jvm_dll_dir]
elif platform.system() == 'Darwin':
    include_dirs += [os.path.join(jdk_home_dir, 'include', 'darwin')]
    library_dirs += [os.path.join(sys.exec_prefix, 'lib')]
    extra_link_args += ['-Xlinker', '-rpath', jvm_dll_dir]

with open('README.rst') as file:
    long_description = file.read()

with open('CHANGES.txt') as file:
    changelog = file.read()

dist=setup(name='jpy',
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
      package_dir={'': os.path.join('src', 'main', 'python')},
      py_modules=['jpyutil'],
      ext_modules=[Extension('jpy',
                             sources,
                             include_dirs=include_dirs,
                             library_dirs=library_dirs,
                             libraries=libraries,
                             extra_link_args=extra_link_args,
                             extra_compile_args=extra_compile_args,
                             define_macros=define_macros,
                             depends=headers),
                   Extension('jdl',
                             sources = [os.path.join(src_main_c_dir, 'jni/org_jpy_DL.c')],
                             include_dirs=include_dirs,
                             library_dirs=library_dirs,
                             libraries=libraries,
                             extra_link_args=extra_link_args,
                             extra_compile_args=extra_compile_args,
                             define_macros=define_macros,
                             )
      ]
)

if 'install' in sys.argv:
    jpy_dll_path = None
    jdl_dll_path = None

    with open('setup.out') as f:
        outputs = f.readlines()
        for output in outputs:
            output = output.strip()
            print('output=', output)
            filename = os.path.basename(output)
            if filename.endswith('.so') or filename.endswith('.pyd'):
                if 'jpy' in filename:
                    jpy_dll_path = output
                if 'jdl' in filename:
                    jdl_dll_path = output

    if not jpy_dll_path:
        log.info("importing module jpy in order to retrieve its shared library location...")
        jpyutil.preload_jvm_dll(jvm_dll_path)
        import jpy
        jpy_dll_path = jpy.__file__

    if not jdl_dll_path:
        log.warn("warning: can't find module path for jdl. You will not be able to call Python from Java on Unix platforms.")

    jpy_config_file_path = jpyutil.get_jpy_config_file_path()
    jpy_config = jpyutil.Properties()
    if os.path.exists(jpy_config_file_path):
        jpy_config.load(jpy_config_file_path)
    jpy_config.set_property('jpy.lib', jpy_dll_path)
    jpy_config.set_property('jdl.lib', jdl_dll_path)
    jpy_config.set_property('jvm.lib', jvm_dll_path)
    jpy_config.set_property('python.lib', jpyutil.find_python_dll_path())
    jpy_config.set_property('python.prefix', sys.prefix)
    if os.path.exists(jpy_config_file_path):
        comment = 'Updated by jpy/setup.py on ' + str(datetime.datetime.now())
    else:
        comment = 'Created by jpy/setup.py on ' + str(datetime.datetime.now())
    jpy_config.store(jpy_config_file_path, comment)

    log.info('written jpy configuration to %s' % (jpy_config_file_path,))

    for key in jpy_config.keys:
        print('  ', key, '=', jpy_config.values[key])

    log.info('compiling Java code...')
    os.system('mvn clean test-compile')

    log.info('executing Python unit tests...')
    failures = 0
    for test in python_tests:
        result = os.system(sys.executable + ' ' + test)
        if result != 0:
            failures += 1

    if failures > 0:
        log.error('one or more Python unit tests failed. Installation is likely broken.')
        exit(1)

    log.info("installing compiled Java code...")
    os.system('mvn install')