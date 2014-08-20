#!/usr/bin/env python3

__author__ = "Norman Fomferra, Brockmann Consult GmbH"
__copyright__ = "Copyright (C) 2014 Brockmann Consult GmbH"
__license__ = "GPL v3"
__version__ = "0.8-SNAPSHOT"

import sys
import os
import os.path
import platform
from distutils import log
from distutils.core import setup
from distutils.extension import Extension
import jpyutil

do_build = 'build' in sys.argv
do_install = 'install' in sys.argv
do_maven = False
if '--maven' in sys.argv:
    do_maven = True
    sys.argv.remove('--maven')

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
jdk_home_dir = jpyutil.find_jdk_home_dir()
if jdk_home_dir is None:
    log.error('Error: environment variable "JAVA_HOME" must be set to a JDK (>= v1.6) installation directory')
    exit(1)

log.info(
    'Building a %s-bit library for a %s system with JDK at %s' % (
        '64' if jpyutil.IS64BIT else '32', platform.system(), jdk_home_dir))

jvm_dll_file = jpyutil.find_jvm_dll_file(jdk_home_dir)
if not jvm_dll_file:
    log.error('Error: Cannot find any JVM shared library')
    exit(1)

jvm_dll_dir = os.path.dirname(jvm_dll_file)

include_dirs = [src_main_c_dir, os.path.join(jdk_home_dir, 'include')]
library_dirs = [jvm_dll_dir]
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

with open('README.txt') as file:
    long_description = file.read()

with open('CHANGES.txt') as file:
    changelog = file.read()

dist = setup(name='jpy',
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
             #package_dir={'': os.path.join('src', 'main', 'python')},
             py_modules=['jpyutil'],
             ext_modules=[Extension('jpy',
                                    sources=sources,
                                    depends=headers,
                                    include_dirs=include_dirs,
                                    library_dirs=library_dirs,
                                    libraries=libraries,
                                    extra_link_args=extra_link_args,
                                    extra_compile_args=extra_compile_args,
                                    define_macros=define_macros),
                          Extension('jdl',
                                    sources=[os.path.join(src_main_c_dir, 'jni/org_jpy_DL.c')],
                                    depends=[os.path.join(src_main_c_dir, 'jni/org_jpy_DL.h')],
                                    include_dirs=include_dirs,
                                    library_dirs=library_dirs,
                                    libraries=libraries,
                                    extra_link_args=extra_link_args,
                                    extra_compile_args=extra_compile_args,
                                    define_macros=define_macros),
             ]
)


if do_install:
    import subprocess

    ##
    ## Default jpy Configuration file
    ## todo - for do_build write build-local jpy Configuration file
    ##

    jpy_config_file = jpyutil.get_jpy_config_file()
    code = subprocess.call([sys.executable, 'jpyutil.py', jpy_config_file, jdk_home_dir])


if (do_build or do_install) and do_maven:
    import subprocess
    import shutil
    import sysconfig

    # If we don't install we need add current build output dir to PYTHONPATH
    if do_build and not do_install:
        # Make accessible the build jpy package
        build_dir = os.path.join('build', 'lib.' + sysconfig.get_platform() + '-' + sysconfig.get_python_version())
        os.environ['PYTHONPATH'] = build_dir
        log.info('set PYTHONPATH = ' + build_dir)

    ##
    ## Java compilation with Maven
    ##

    if not os.getenv('JAVA_HOME'):
        os.environ['JAVA_HOME'] = jdk_home_dir

    log.info('Compiling Java code...')
    code = subprocess.call(['mvn', 'clean', 'test-compile'])
    if code:
        exit(code)

    ##
    ## Python unit tests
    ##

    log.info('Executing Python unit tests...')
    failures = 0
    for test in python_tests:
        code = subprocess.call([sys.executable, test])
        if code:
            failures += 1

    if failures > 0:
        log.error(failures + ' Python unit test(s) failed. Installation is likely broken.')
        exit(1)

    ##
    ## Java package or install with Maven
    ##

    log.info("Installing compiled Java code...")
    code = subprocess.call(['mvn', 'install' if 'install' in sys.argv else 'package'])
    if code:
        exit(code)

    ##
    ## Result: lib/jpy.jar
    ##

    jpy_version_jar_filename = 'jpy-' + __version__ + '.jar'
    jpy_plain_jar_filename = 'jpy.jar'
    lib_dir = 'lib'
    if not os.path.exists(lib_dir):
        os.mkdir(lib_dir)
    jar_src_dir = os.path.join('target', jpy_version_jar_filename)
    jar_dst_dir = os.path.join(lib_dir, jpy_plain_jar_filename)
    log.info("Copying " + jar_src_dir + " -> " + jar_dst_dir + "")
    shutil.copy(jar_src_dir, jar_dst_dir)
