# !/usr/bin/env python3

__author__ = "Norman Fomferra, Brockmann Consult GmbH"
__copyright__ = "Copyright (C) 2015 Brockmann Consult GmbH"
__license__ = "GPL v3"
__version__ = "0.8"

import sys
import os
import os.path
import platform
import subprocess
import shutil
import sysconfig
import pprint

from distutils import log
from distutils.core import setup
from distutils.extension import Extension

base_dir = os.path.dirname(os.path.abspath(__file__))
src_main_c_dir = os.path.join(base_dir, 'src', 'main', 'c')
src_main_py_dir = os.path.join(base_dir, 'src', 'main', 'python')
src_test_py_dir = os.path.join(base_dir, 'src', 'test', 'python')

sys.path = [src_main_py_dir] + sys.path
import jpyutil

print('Using ' + jpyutil.__file__)

do_maven = False
if '--maven' in sys.argv:
    do_maven = True
    sys.argv.remove('--maven')
else:
    print('Note that you can use non-standard global option [--maven] '
          'to force a Java Maven build for the jpy Java API')

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

# e.g. jdk_home_dir = '/home/marta/jdk1.7.0_15'
jdk_home_dir = jpyutil.find_jdk_home_dir()
if jdk_home_dir is None:
    log.error('Error: environment variable "JAVA_HOME" must be set to a JDK (>= v1.7) installation directory')
    exit(1)

log.info('Building a %s-bit library for a %s system with JDK at %s' % (
    '64' if jpyutil.PYTHON_64BIT else '32', platform.system(), jdk_home_dir))

jvm_dll_file = jpyutil.find_jvm_dll_file(jdk_home_dir)
if not jvm_dll_file:
    log.error('Error: Cannot find any JVM shared library')
    exit(1)

lib_dir = os.path.join(base_dir, 'lib')
jpy_jar_file = os.path.join(lib_dir, 'jpy.jar')

if do_maven:

    #
    # Java packaging with Maven
    #

    if not os.getenv('JAVA_HOME'):
        # make sure Maven uses the same JDK which we have used to compile and link the C-code
        os.environ['JAVA_HOME'] = jdk_home_dir

    goal = 'package'
    log.info("Executing Maven goal '" + goal + "'")
    code = subprocess.call(['mvn', 'clean', goal, '-DskipTests'], shell=platform.system() == 'Windows')
    if code:
        exit(code)

    #
    # Copy result to lib/jpy.jar
    #

    if not os.path.exists(lib_dir):
        os.mkdir(lib_dir)
    built_jpy_jar_file = os.path.join('target', 'jpy-' + __version__ + '.jar')
    log.info("Copying " + built_jpy_jar_file + " -> " + jpy_jar_file + "")
    shutil.copy(built_jpy_jar_file, jpy_jar_file)


#
# Prepare setup arguments and call setup
#

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

with open('README.md') as file:
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
                          ],
             classifiers=[
                 # How mature is this project? Common values are
                 #   3 - Alpha
                 #   4 - Beta
                 #   5 - Production/Stable
                 'Development Status :: 4 - Beta',

                 # Indicate who your project is intended for
                 'Intended Audience :: Developers',

                 # Pick your license as you wish (should match "license" above)
                 'License :: OSI Approved :: GPL 3 License',

                 # Specify the Python versions you support here. In particular, ensure
                 # that you indicate whether you support Python 2, Python 3 or both.
                 'Programming Language :: Python :: 2',
                 'Programming Language :: Python :: 2.7',
                 'Programming Language :: Python :: 3',
                 'Programming Language :: Python :: 3.3',
                 'Programming Language :: Python :: 3.4',
             ]
             )

# pprint.pprint(dist.commands)
# pprint.pprint(dist.command_obj)
# pprint.pprint(dist.command_obj['build'].__dict__)
# pprint.pprint(dist.command_obj['install'].__dict__)

#
# Continue with custom setup if any commands including a build have been provided
#

if dist.commands and len(dist.commands) > 0 \
        and dist.command_obj and len(dist.command_obj) > 0 \
        and 'clean' not in dist.command_obj \
        and 'build' in dist.command_obj:

    #
    # Get build directory (whose content we'll zip and in which we test)
    #

    build_dir = dist.command_obj['build'].build_lib
    if not os.path.exists(build_dir):
        log.error("Missing build directory '" + build_dir + "'")
        exit(1)

    #
    # Get install directory
    #

    install_dir = None
    if 'install' in dist.command_obj:
        install_dir = dist.command_obj['install'].install_lib
        if not os.path.exists(install_dir):
            log.warn("Missing install directory '" + install_dir + "'")
            install_dir = None

    #
    # Get distribution file list from build directory.
    #

    dist_name = 'jpy.' + sysconfig.get_platform() + '-' + sysconfig.get_python_version()
    dist_files = jpyutil._list_dir_entries(build_dir,
                                           excludes=['__pycache__/',
                                                     'jpyconfig.properties',
                                                     'jpyconfig.py',
                                                     'jpyconfig.pyc'])

    #
    # Write jpy version info (jpy.<platform>-<python-version>.info) to target directories
    #

    def _write_version_info(version_file, jpy_info_dict):
        log.info('Writing jpy version info to ' + version_file)
        with open(version_file, 'w') as f:
            f.write(pprint.pformat(jpy_info_dict))

    version_filename = dist_name + ".info"
    jpy_info = {'jpy_version': __version__,
                'jpy_files': dist_files + [version_filename]}

    _write_version_info(os.path.join(build_dir, version_filename), jpy_info)
    if install_dir and install_dir != build_dir:
        _write_version_info(os.path.join(install_dir, version_filename), jpy_info)

    #
    # Zip the plain target directory contents (similar to 'bdist' but without install path info)
    #

    dist_filename = dist_name + '.zip'
    archive_file = os.path.join('build', dist_filename)
    jpyutil._zip_entries(archive_file, build_dir, dist_files, verbose=True)

    #
    # Write jpy configuration files to target directories
    #

    def _write_jpy_config(target_dir, jvm_dll_file, jdk_home_dir):
        log.info('Writing jpy configuration to ' + target_dir)
        return subprocess.call([sys.executable,
                                os.path.join(target_dir, 'jpyutil.py'),
                                '--out', target_dir,
                                '--jvm_dll', jvm_dll_file,
                                '--java_home', jdk_home_dir,
                                '--log_level', 'DEBUG',
                                '--req_java',
                                '--req_py'])

    code = _write_jpy_config(build_dir, jvm_dll_file, jdk_home_dir)
    if code != 0:
        exit(code)
    if install_dir and install_dir != build_dir:
        code = _write_jpy_config(install_dir, jvm_dll_file, jdk_home_dir)
        if code != 0:
            exit(code)

    #
    # Ensure that the build directory is on Python's 'sys.path' when invoking
    # Python via subprocess.call([sys.executable, ...]) from this script.
    #

    class EnvVar:
        def __init__(self, name):
            self.name = name
            self.changed = False
            self.saved_value = self.get_value()

        def __del__(self):
            self.restore()

        def get_value(self):
            return os.environ.get(self.name)

        def set_value(self, value):
            self._set_value(value)
            self.changed = True

        def _set_value(self, value):
            log.info("setting environment variable '" + self.name + "' to " + repr(value))
            if value:
                os.environ[self.name] = value
            else:
                os.environ.pop(self.name)

        def prepend_path(self, path):
            old_value = self.get_value()
            new_value = os.pathsep.join([path, old_value]) if old_value else path
            self.set_value(new_value)

        def restore(self):
            if self.changed:
                self._set_value(self.saved_value)
                self.changed = False

    pp = EnvVar('PYTHONPATH')

    #
    # Python unit tests with Java runtime classes
    #

    log.info('Executing Python unit tests (against Java runtime classes)...')
    pp.prepend_path(build_dir)
    fails = jpyutil._execute_python_scripts(python_java_rt_tests)
    pp.restore()
    if fails > 0:
        log.error(str(fails) + ' Python unit test(s) failed. Installation is likely broken.')
        exit(1)

    if do_maven:

        #
        # Python unit tests with jpy test classes built by Maven
        #

        log.info('Executing Python unit tests (against jpy test classes)...')
        pp.prepend_path(build_dir)
        fails = jpyutil._execute_python_scripts(python_java_jpy_tests)
        pp.restore()
        if fails > 0:
            log.error(str(fails) + ' Python unit test(s) failed. Installation is likely broken.')
            exit(1)

        #
        # Java install or test with Maven. Goal 'package' has already been achieved.
        #

        if 'install' in dist.commands:
            goal = 'install'
        else:
            goal = 'test'
        arg_line = '-DargLine=-Xmx512m -Djpy.config=' + os.path.join(build_dir,
                                                                     'jpyconfig.properties') + ' -Djpy.debug=true'
        log.info("Executing Maven goal " + repr(goal) + " with arg line " + repr(arg_line))
        code = subprocess.call(['mvn', goal, arg_line], shell=platform.system() == 'Windows')
        if code:
            exit(code)

    log.info("jpy binary build is '" + archive_file + "'")
    if install_dir:
        log.info("jpy installed into '" + install_dir + "'")
