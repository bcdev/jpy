import sys
import sysconfig
import os.path
import platform
import ctypes
import ctypes.util
import logging


def _get_python_lib_name():
    try:
        abiflags = sys.abiflags
    except AttributeError:
        abiflags = ''
    version = sysconfig.get_config_var('VERSION')
    if not version:
        version = ''
    return 'python' + version + abiflags


PYTHON_64BIT = sys.maxsize > 2 ** 32
PYTHON_LIB_NAME = _get_python_lib_name()
JVM_LIB_NAME = 'jvm'


def _get_unique_config_values(names):
    values = []
    for name in names:
        value = sysconfig.get_config_var(name)
        if value and not value in values:
            values.append(value)
    return values


def _add_paths_if_exists(path_list, *paths):
    for path in paths:
        if os.path.exists(path) and not path in path_list:
            path_list.append(path)
    return path_list


def _get_module_path(name, fail=False):
    import imp

    module = imp.find_module(name)
    if not module and fail:
        raise RuntimeError("can't find module '" + name + "'")
    path = module[1]
    if not path and fail:
        raise RuntimeError("module '" + name + "' is missing a file path")
    return path


def _find_file(search_dirs, *filenames):
    for dir in search_dirs:
        dir = os.path.normpath(dir)
        for filename in filenames:
            path = os.path.join(dir, filename)
            path_exists = os.path.exists(path)
            logging.debug("Exists '%s'? %s" % (path, "yes" if path_exists else "no"))
            if path_exists:
                return path
    return None


def _get_java_api_properties(fail=False):
    jpy_config = Properties()
    jpy_config.set_property('jpy.jpyLib', _get_module_path('jpy', fail=fail))
    jpy_config.set_property('jpy.jdlLib', _get_module_path('jdl', fail=fail))
    jpy_config.set_property('jpy.pythonLib', _find_python_dll_file(fail=fail))
    jpy_config.set_property('jpy.pythonPrefix', sys.prefix)
    jpy_config.set_property('jpy.pythonExecutable', sys.executable)
    return jpy_config


def find_jdk_home_dir():
    for name in ('JPY_JAVA_HOME', 'JPY_JDK_HOME', 'JAVA_HOME', 'JDK_HOME', ):
        jdk_home_dir = os.environ.get(name, None)
        if jdk_home_dir \
                and os.path.exists(os.path.join(jdk_home_dir, 'include')) \
                and os.path.exists(os.path.join(jdk_home_dir, 'lib')):
            return jdk_home_dir
    return None


def find_jvm_dll_file(java_home_dir=None, fail=False):
    """
    Try to detect JVM DLL file.
    :param java_home_dir:
    :return: pathname if found, else None
    """

    logging.debug("Searching for JVM shared library file")

    if java_home_dir:
        jvm_dll_path = _find_jvm_dll_file(java_home_dir)
        if jvm_dll_path:
            return jvm_dll_path

    jvm_dll_path = os.environ.get('JPY_JVM_DLL', None)
    if jvm_dll_path:
        return jvm_dll_path

    for name in ('JPY_JAVA_HOME', 'JPY_JDK_HOME', 'JPY_JRE_HOME', 'JAVA_HOME', 'JDK_HOME', 'JRE_HOME', 'JAVA_JRE'):
        java_home_dir = os.environ.get(name, None)
        if java_home_dir:
            jvm_dll_path = _find_jvm_dll_file(java_home_dir)
            if jvm_dll_path:
                return jvm_dll_path

    jvm_dll_path = ctypes.util.find_library(JVM_LIB_NAME)
    if not jvm_dll_path and fail:
        raise RuntimeError("can't find any JVM shared library")

    return jvm_dll_path


def _get_jvm_lib_dirs(java_home_dir):
    arch = 'amd64' if PYTHON_64BIT else 'i386'
    return (os.path.join(java_home_dir, 'bin'),
            os.path.join(java_home_dir, 'bin', 'server'),
            os.path.join(java_home_dir, 'bin', 'client'),
            os.path.join(java_home_dir, 'bin', arch),
            os.path.join(java_home_dir, 'bin', arch, 'server'),
            os.path.join(java_home_dir, 'bin', arch, 'client'),
            os.path.join(java_home_dir, 'lib'),
            os.path.join(java_home_dir, 'lib', 'server'),
            os.path.join(java_home_dir, 'lib', 'client'),
            os.path.join(java_home_dir, 'lib', arch),
            os.path.join(java_home_dir, 'lib', arch, 'server'),
            os.path.join(java_home_dir, 'lib', arch, 'client'),
    )


def _find_jvm_dll_file(java_home_dir):
    logging.debug("Searching for JVM shared library file in %s" % repr(java_home_dir))

    if not os.path.exists(java_home_dir):
        return None

    search_dirs = []
    jre_home_dir = os.path.join(java_home_dir, 'jre')
    if os.path.exists(jre_home_dir):
        search_dirs += _get_jvm_lib_dirs(jre_home_dir)
    search_dirs += _get_jvm_lib_dirs(java_home_dir)

    search_dirs = _add_paths_if_exists([], *search_dirs)

    if platform.system() == 'Windows':
        return _find_file(search_dirs, 'jvm.dll')
    elif platform.system() == 'Darwin':
        return _find_file(search_dirs, 'libjvm.dylib')

    # 'Window' and 'Darwin' did not succeed, try 'libjvm.so' on remaining platforms
    return _find_file(search_dirs, 'libjvm.so')


def _find_python_dll_file(fail=False):

    logging.debug("Searching for Python shared library file")

    filenames = _get_unique_config_values(('LDLIBRARY', 'INSTSONAME', 'PY3LIBRARY', 'DLLLIBRARY',))
    search_dirs = _get_unique_config_values(('LDLIBRARYDIR', 'srcdir', 'BINDIR', 'DESTLIB', 'DESTSHARED',
                                             'BINLIBDEST', 'LIBDEST', 'LIBDIR', 'MACHDESTLIB',))

    search_dirs.append(sys.prefix)

    if platform.system() == 'Windows':
        filenames += ['python' + str(sys.version_info.major) + str(sys.version_info.minor) + '.dll',
                      'python' + str(sys.version_info.major) + '.dll',
                      'python.dll']
        lib_dirs_extra = [os.path.join(lib, 'DLLs') for lib in search_dirs]
        search_dirs = lib_dirs_extra + search_dirs
    else:
        multiarchsubdir = sysconfig.get_config_var('multiarchsubdir')
        if multiarchsubdir:
            while multiarchsubdir.startswith('/'):
                multiarchsubdir = multiarchsubdir[1:]
            lib_dirs_extra = [os.path.join(lib, multiarchsubdir) for lib in search_dirs]
            search_dirs = lib_dirs_extra + search_dirs

    search_dirs = _add_paths_if_exists([], *search_dirs)

    # pprint.pprint(search_dirs)
    # pprint.pprint(filenames)

    python_dll_path = _find_file(search_dirs, *filenames)
    if not python_dll_path:
        python_dll_path = ctypes.util.find_library(PYTHON_LIB_NAME)

    if not python_dll_path and fail:
        raise RuntimeError("can't find any Python shared library")

    return python_dll_path


def _read_config(config_file):
    config = Config()
    config.load(config_file)
    return config


def _get_python_api_config(config_file=None):
    if config_file:
        # 1. Try argument, if any
        return _read_config(config_file)

    try:
        # 2. Try Python import machinery
        import jpyconfig

        return jpyconfig

    except ImportError:
        # 3. Try 'JPY_PY_CONFIG' environment variable, if any
        config_file = os.environ.get('JPY_PY_CONFIG', None)
        if config_file:
            return _read_config(config_file)

    return None


def preload_jvm_dll(jvm_dll_file=None, java_home_dir=None):
    if not jvm_dll_file:
        jvm_dll_file = find_jvm_dll_file(java_home_dir=java_home_dir, fail=True)
    logging.debug('Preloading JVM shared library %s' % repr(jvm_dll_file))
    return ctypes.CDLL(jvm_dll_file, mode=ctypes.RTLD_GLOBAL)


def get_jvm_options(jvm_maxmem=None,
                    jvm_classpath=None,
                    jvm_properties=None,
                    jvm_options=None,
                    config=None):
    if config:
        if not jvm_maxmem:
            jvm_maxmem = getattr(config, 'jvm_maxmem', None)
        if not jvm_classpath:
            jvm_classpath = getattr(config, 'jvm_classpath', None)
        if not jvm_properties:
            jvm_properties = getattr(config, 'jvm_properties', None)
        if not jvm_options:
            jvm_options = getattr(config, 'jvm_options', None)

    jvm_cp = None
    if jvm_classpath and len(jvm_classpath) > 0:
        jvm_cp = os.pathsep.join(jvm_classpath)
    if not jvm_cp:
        jvm_cp = os.environ.get('JPY_JVM_CLASSPATH', None)

    if not jvm_maxmem:
        jvm_maxmem = os.environ.get('JPY_JVM_MAXMEM', None)

    java_api_properties = _get_java_api_properties().values
    if jvm_properties:
        # Overwrite jpy_config
        jvm_properties = dict(list(java_api_properties.items()) + list(jvm_properties.items()))
    else:
        jvm_properties = java_api_properties

    options = []
    if jvm_maxmem:
        options.append('-Xmx' + jvm_maxmem)
    if jvm_cp:
        options.append('-Djava.class.path=' + jvm_cp)
    if jvm_properties:
        for key in jvm_properties:
            value = jvm_properties[key]
            options.append('-D' + key + '=' + value)
    if jvm_options:
        options += jvm_options

    return options


def init_jvm(java_home=None,
             jvm_dll=None,
             jvm_maxmem=None,
             jvm_classpath=None,
             jvm_properties=None,
             jvm_options=None,
             config_file=None,
             config=None):
    if not config:
        config = _get_python_api_config(config_file=config_file)

    if config:
        if not java_home:
            java_home = getattr(config, 'java_home', None)
        if not jvm_dll:
            jvm_dll = getattr(config, 'jvm_dll', None)

    if not jvm_dll:
        jvm_dll = find_jvm_dll_file(java_home_dir=java_home)

    if jvm_dll:
        cdll = preload_jvm_dll(jvm_dll)
    else:
        cdll = None

    import jpy

    if not jpy.has_jvm():
        jvm_options = get_jvm_options(jvm_maxmem=jvm_maxmem,
                                      jvm_classpath=jvm_classpath,
                                      jvm_properties=jvm_properties,
                                      jvm_options=jvm_options,
                                      config=config)
        logging.debug('Creating JVM with options %s' % repr(jvm_options))
        jpy.create_jvm(options=jvm_options)
    else:
        jvm_options = None

    # print('jvm_dll =', jvm_dll)
    # print('jvm_options =', jvm_options)
    return cdll, jvm_options


class Config:
    def load(self, path):
        """
        Read Python file from 'path', execute it and return object that stores all variables of the Python code as attributes.
        :param path:
        :return:
        """
        with open(path) as f:
            code = f.read()
            exec (code, {}, self.__dict__)


class Properties:
    def __init__(self, values=None):
        if values:
            self.keys = values.keys()
            self.values = values.copy()
        else:
            self.keys = []
            self.values = {}


    def set_property(self, key, value):
        if value:
            if not key in self.keys:
                self.keys.append(key)
            self.values[key] = value
        else:
            if key in self.keys:
                self.keys.remove(key)
                self.values.pop(key)


    def get_property(self, key, default_value=None):
        return self.values[key] if key in self.values else default_value


    def store(self, path, comments=()):
        with open(path, 'w') as f:
            for comment in comments:
                f.write('# ' + str(comment).replace('\\', '\\\\') + '\n')
            for key in self.keys:
                value = self.get_property(key)
                if value:
                    f.write(str(key) + ' = ' + str(value).replace('\\', '\\\\') + '\n')
                else:
                    f.write(str(key) + ' =\n')


    def load(self, path):
        self.__init__()
        with open(path) as f:
            lines = f.readlines()
            for line in lines:
                if line and len(line) > 0 and not line.startswith('#'):
                    tokens = line.split('=')
                    if len(tokens) == 2:
                        self.set_property(tokens[0].strip(), tokens[1].strip().replace('\\\\', '\\'))
                    else:
                        raise ValueError('illegal Java properties format ' + line)


def _execute_python_scripts(scripts):
    import subprocess

    failures = 0
    for script in scripts:
        exit_code = subprocess.call([sys.executable, script])
        if exit_code:
            failures += 1
    return failures


def __contains(list, entry):
    for item in list:
        if item == entry or (item.endswith('/') and entry.startswith(item)):
            return True
    return False


def _list_dir_entries(rootdir, includes=None, excludes=None):
    rootoffs = len(rootdir) + 1
    entries = []
    for (dirpath, dirnames, filenames) in os.walk(rootdir):
        for filename in filenames:
            path = os.path.join(dirpath, filename)
            entry = path[rootoffs:].replace(os.sep, '/')
            included = not includes or __contains(includes, entry)
            excluded = not excludes or __contains(excludes, entry)
            if included and not excluded:
                entries.append(entry)
    return entries


def _zip_entries(archive_path, dir, dir_entries, verbose=False):
    import zipfile

    if verbose: print("creating '" + archive_path + "' and adding '" + dir + "' to it")
    with zipfile.ZipFile(archive_path, 'w') as archive:
        for entry in dir_entries:
            path = os.path.join(dir, entry.replace('/', os.sep))
            archive.write(path, arcname=entry)
            if verbose: print("adding '" + entry + "'")


def write_config_files(out_dir='.',
                       java_home_dir=None,
                       jvm_dll_file=None,
                       req_java_api_conf=True,
                       req_py_api_conf=True):
    import datetime

    retcode = 0

    tool_name = os.path.basename(__file__)

    py_api_config_basename = 'jpyconfig.py'
    java_api_config_basename = 'jpyconfig.properties'

    if not jvm_dll_file:
        jvm_dll_file = find_jvm_dll_file(java_home_dir=java_home_dir)
    if jvm_dll_file:
        py_api_config_file = os.path.join(out_dir, py_api_config_basename)
        try:
            with open(py_api_config_file, 'w') as f:
                f.write("# Created by '%s' tool on %s\n" % (tool_name, str(datetime.datetime.now())))
                f.write("# This file is read by the 'jpyutil' module in order load and configure the JVM from Python\n")
                if java_home_dir:
                    f.write('java_home = %s\n' % repr(java_home_dir))
                f.write('jvm_dll = %s\n' % repr(jvm_dll_file))
                f.write('jvm_maxmem = None\n')
                f.write('jvm_classpath = []\n')
                f.write('jvm_properties = {}\n')
                f.write('jvm_options = []\n')
            logging.info("jpy Python API configuration written to '%s'" % py_api_config_file)
        except Exception:
            logging.exception("Error while writing Python API configuration")
            if req_py_api_conf:
                retcode = 1
    else:
        logging.error("Can't determine any JVM shared library")
        if req_py_api_conf:
            retcode = 2

    try:
        java_api_config_file = os.path.join(out_dir, java_api_config_basename)
        java_api_properties = _get_java_api_properties(fail=req_java_api_conf)
        java_api_properties.store(java_api_config_file, comments=[
            "Created by '%s' tool on %s" % (tool_name, str(datetime.datetime.now())),
            "This file is read by the jpy Java API (org.jpy.PyLib class) in order to find shared libraries"])
        logging.info("jpy Java API configuration written to '%s'" % java_api_config_file)
    except Exception:
        logging.exception("Error while writing Java API configuration")
        if req_java_api_conf:
            retcode = 3

    return retcode


def _main():
    import argparse


    parser = argparse.ArgumentParser(description='Generate configuration files for the jpy Python API (jpyconfig.py)\n'
                                                 'and the jpy Java API (jpyconfig.properties).')
    parser.add_argument("-o", "--out", action='store', default='.',
                        help="output directory for the configuration files")
    parser.add_argument("--java_home", action='store', default=None, help="Java home directory")
    parser.add_argument("--jvm_dll", action='store', default=None, help="Java shared library location")
    parser.add_argument("--log_file", action='store', default=None, help="Log file")
    parser.add_argument("--log_level", action='store', default='INFO',
                        help="Possible values: DEBUG, INFO, WARNING, ERROR")
    parser.add_argument("-j", "--req_java", action='store_true', default=False,
                        help="require that Java API configuration succeeds")
    parser.add_argument("-p", "--req_py", action='store_true', default=False,
                        help="require that Python API configuration succeeds")
    args = parser.parse_args()

    log_level = getattr(logging, args.log_level.upper(), None)
    if not isinstance(log_level, int):
        raise ValueError('Invalid log level: %s' % log_level)

    log_format='%(levelname)s: %(message)s'
    log_file = args.log_file
    if log_file:
        logging.basicConfig(format=log_format, level=log_level, filename=log_file, filemode='w')
    else:
        logging.basicConfig(format=log_format, level=log_level)

    try:
        return write_config_files(out_dir=args.out,
                                  java_home_dir=args.java_home,
                                  jvm_dll_file=args.jvm_dll,
                                  req_java_api_conf=args.req_java,
                                  req_py_api_conf=args.req_py)
    except Exception:
        logging.exception("Configuration failed")
        return 10


if __name__ == '__main__':
    _main()

