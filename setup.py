#!/usr/bin/env python3

__author__ = "Norman Fomferra, Brockmann Consult GmbH"
__copyright__ = "Copyright (C) 2014 Brockmann Consult GmbH"
__license__ = "GPL v3"
__version__ = "0.8-SNAPSHOT"

import sys
import os
import os.path
import platform
import subprocess
import shutil
import sysconfig
from distutils import log
from distutils.core import setup
from distutils.extension import Extension

src_main_c_dir = os.path.join('src', 'main', 'c')
src_main_py_dir = os.path.join('src', 'main', 'python')
src_test_py_dir = os.path.join('src', 'test', 'python')

sys.path = [src_main_py_dir] + sys.path
import jpyutil

do_build = 'build' in sys.argv
do_install = 'install' in sys.argv
do_maven = False
if '--maven' in sys.argv:
    do_maven = True
    sys.argv.remove('--maven')

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

# Python unit tests that just use Java runtime classes (rt.jar)
python_java_rt_tests = [
    os.path.join(src_test_py_dir, 'jpy_rt_test.py'),
    os.path.join(src_test_py_dir, 'jpy_mt_test.py'),
    os.path.join(src_test_py_dir, 'jpy_diag_test.py'),
    #os.path.join(src_test_py_dir, 'jpy_perf_test.py'),
]

# Python unit tests that require jpy test fixture classes to be accessible
# via JRE system property '-Djava.class.path=target/test-classes'
python_java_jpy_tests = [
    os.path.join(src_test_py_dir, 'jpy_array_test.py'),
    os.path.join(src_test_py_dir, 'jpy_field_test.py'),
    os.path.join(src_test_py_dir, 'jpy_retval_test.py'),
    os.path.join(src_test_py_dir, 'jpy_exception_test.py'),
    os.path.join(src_test_py_dir, 'jpy_overload_test.py'),
    os.path.join(src_test_py_dir, 'jpy_typeconv_test.py'),
    os.path.join(src_test_py_dir, 'jpy_typeres_test.py'),
    os.path.join(src_test_py_dir, 'jpy_modretparam_test.py'),
    os.path.join(src_test_py_dir, 'jpy_gettype_test.py'),
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
             package_dir={'': src_main_py_dir},
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


def _execute_python_scripts(scripts):
    failures = 0
    for script in scripts:
        exit_code = subprocess.call([sys.executable, script])
        if exit_code:
            failures += 1
    return failures


if do_build or do_install:
    build_dir = os.path.join('build', 'lib.' + sysconfig.get_platform() + '-' + sysconfig.get_python_version())

    # If we don't install we need add current build output dir to PYTHONPATH
    if do_build and not do_install:
        # Make accessible the build jpy package
        os.environ['PYTHONPATH'] = build_dir
        log.info('set PYTHONPATH = ' + build_dir)

    ##
    ## Write jpy configuration file
    ##

    if do_install:
        jpy_config_file = jpyutil.get_jpy_user_config_file()
    else:
        jpy_config_file = os.path.join(build_dir, 'jpy.properties')
    log.info('Writing jpy configuration to ' + jpy_config_file)
    code = subprocess.call([sys.executable, os.path.join(src_main_py_dir, 'jpyutil.py'), jpy_config_file, jdk_home_dir])
    if code:
        exit(code)

    ##
    ## Python unit tests with Java runtime classes
    ##

    log.info('Executing Python unit tests (against Java runtime classes)...')
    fails = _execute_python_scripts(python_java_rt_tests)
    if fails > 0:
        log.error(str(fails) + ' Python unit test(s) failed. Installation is likely broken.')
        exit(1)

if (do_build or do_install) and do_maven:

    ##
    ## Java compilation with Maven
    ##

    if not os.getenv('JAVA_HOME'):
        # make sure Maven uses the same JDK which we have used to compile and link the C-code
        os.environ['JAVA_HOME'] = jdk_home_dir

    log.info('Compiling Java code...')
    code = subprocess.call(['mvn', 'clean', 'test-compile'], shell=True)
    if code:
        exit(code)

    ##
    ## Python unit tests with jpy test classes
    ##

    log.info('Executing Python unit tests (against jpy test classes)...')
    fails = _execute_python_scripts(python_java_jpy_tests)
    if fails > 0:
        log.error(str(fails) + ' Python unit test(s) failed. Installation is likely broken.')
        exit(1)

    ##
    ## Java package or install with Maven
    ##

    log.info("Installing compiled Java code...")
    if do_install:
        goal = 'install'
    else:
        goal = 'package'
    code = subprocess.call(['mvn', goal], shell=True)
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
