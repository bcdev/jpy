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

print('Using ' + jpyutil.__file__)

do_maven = False
if '--maven' in sys.argv:
    do_maven = True
    sys.argv.remove('--maven')
else:
    print(
        'Note that you can use non-standard global option [--maven] to force a Java Maven build incl. jpy Java API testing')

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
    # os.path.join(src_test_py_dir, 'jpy_perf_test.py'),
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

log.info('Building a %s-bit library for a %s system with JDK at %s' % (
    '64' if jpyutil.PYTHON_64BIT else '32', platform.system(), jdk_home_dir))

jvm_dll_file = jpyutil.find_jvm_dll_file(jdk_home_dir)
if not jvm_dll_file:
    log.error('Error: Cannot find any JVM shared library')
    exit(1)

lib_dir = 'lib'
jpy_jar_file = os.path.join(lib_dir, 'jpy.jar')

if do_maven:

    ##
    ## Java packaging with Maven
    ##

    if not os.getenv('JAVA_HOME'):
        # make sure Maven uses the same JDK which we have used to compile and link the C-code
        os.environ['JAVA_HOME'] = jdk_home_dir

    goal = 'package'
    log.info("Executing Maven goal '" + goal + "'")
    code = subprocess.call(['mvn', 'clean', goal, '-DskipTests'], shell=platform.system() == 'Windows')
    if code:
        exit(code)

    ##
    ## Copy result to lib/jpy.jar
    ##

    if not os.path.exists(lib_dir):
        os.mkdir(lib_dir)
    built_jpy_jar_file = os.path.join('target', 'jpy-' + __version__ + '.jar')
    log.info("Copying " + built_jpy_jar_file + " -> " + jpy_jar_file + "")
    shutil.copy(built_jpy_jar_file, jpy_jar_file)


##
## Prepare setup arguments and call setup
##

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
             package_data=[('', [jpy_jar_file])],
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

##
## Continue with custom setup if any commands including a build have been provided
##

if dist.commands and len(dist.commands) > 0 \
        and dist.command_obj and len(dist.command_obj) > 0 \
        and not 'clean' in dist.command_obj \
        and 'build' in dist.command_obj:

    # import pprint
    # pprint.pprint(dist.commands)
    # pprint.pprint(dist.command_obj)
    # pprint.pprint(dist.command_obj['build'].__dict__)
    # pprint.pprint(dist.command_obj['install'].__dict__)

    ##
    ## Get target directory (whose content we'll zip in which we test)
    ##

    # Explicit 'install' command
    if 'install' in dist.commands and 'install' in dist.command_obj:
        target_dir = dist.command_obj['install'].install_lib
    # Explicit or implicit 'build' command
    elif 'build' in dist.command_obj:
        target_dir = dist.command_obj['build'].build_lib
    else:
        # Nothing to do
        exit(0)

    dist_name = 'jpy.' + sysconfig.get_platform() + '-' + sysconfig.get_python_version()
    target_dir, dist_files = jpyutil._list_dir_entries(target_dir,
                                                       excludes=['__pycache__/',
                                                                 'jpyconfig.properties',
                                                                 'jpyconfig.py'])

    ##
    ## Ensure that the new 'jpy' from target directory is found first
    ##

    pp = os.environ.get('PYTHONPATH')
    os.environ['PYTHONPATH'] = os.pathsep.join([target_dir, pp]) if pp else target_dir

    ##
    ## Write jpy version info to target directory
    ##

    jpy_version_file = os.path.join(target_dir, dist_name + ".info")
    log.info('Writing jpy version info to ' + jpy_version_file)
    import glob

    jpy_files = glob.glob(os.path.join(target_dir, '*'))
    jpy_info = {'jpy_version': __version__,
                'jpy_files': dist_files}
    import pprint

    with open(jpy_version_file, 'w') as f:
        f.write(pprint.pformat(jpy_info))

    ##
    ## Write jpy configuration files to target directory
    ##

    log.info('Writing jpy configuration to ' + target_dir)
    code = subprocess.call([sys.executable,
                            os.path.join(target_dir, 'jpyutil.py'),
                            '--out', target_dir,
                            '--jvm_dll', jvm_dll_file,
                            '--java_home', jdk_home_dir, '-f'])
    if code:
        exit(code)

    ##
    ## Zip the plain target directory contents (similar to 'bdist' but without install path info)
    ##

    archive_file = os.path.join('build', dist_name + '.zip')
    jpyutil._zip_entries(archive_file, target_dir, dist_files, verbose=True)

    ##
    ## Python unit tests with Java runtime classes
    ##

    log.info('Executing Python unit tests (against Java runtime classes)...')
    fails = jpyutil._execute_python_scripts(python_java_rt_tests)
    if fails > 0:
        log.error(str(fails) + ' Python unit test(s) failed. Installation is likely broken.')
        exit(1)

    if do_maven:

        ##
        ## Python unit tests with jpy test classes built by Maven
        ##

        log.info('Executing Python unit tests (against jpy test classes)...')
        fails = jpyutil._execute_python_scripts(python_java_jpy_tests)
        if fails > 0:
            log.error(str(fails) + ' Python unit test(s) failed. Installation is likely broken.')
            exit(1)

        ##
        ## Java install or test with Maven. Goal package has already been done..
        ##

        if 'install' in dist.commands:
            goal = 'install'
        else:
            goal = 'test'
        log.info("Executing Maven goal '" + goal + "'")
        arg_line = '-DargLine=-Xmx512m -Djpy.config=' + os.path.join(target_dir, 'jpyconfig.properties') + ''
        code = subprocess.call(['mvn', goal, arg_line], shell=platform.system() == 'Windows')
        if code:
            exit(code)

    log.info("Successful build is in " + target_dir)
