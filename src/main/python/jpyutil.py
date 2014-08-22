import sys
import sysconfig
import os.path
import platform
import ctypes
import ctypes.util


def _get_python_lib_name():
    try:
        abiflags = sys.abiflags
    except AttributeError:
        abiflags = ''
    return 'python' + sysconfig.get_config_var('VERSION') + abiflags


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
            if os.path.exists(path):
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
    if java_home_dir:
        jvm_dll_path = _find_jvm_dll_file(java_home_dir)
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
    if not os.path.exists(java_home_dir):
        return None

    search_dirs = []
    jre_home_dir = os.path.join(java_home_dir, 'jre')
    if os.path.exists(jre_home_dir):
        search_dirs += _get_jvm_lib_dirs(jre_home_dir)
    search_dirs += _get_jvm_lib_dirs(java_home_dir)

    search_dirs = _add_paths_if_exists([], *search_dirs)

    if platform.system() is 'Windows':
        return _find_file(search_dirs, 'jvm.dll')
    else:
        return _find_file(search_dirs, 'libjvm.so')


def _find_python_dll_file(fail=False):
    filenames = _get_unique_config_values(('LDLIBRARY', 'INSTSONAME', 'PY3LIBRARY', 'DLLLIBRARY',))
    search_dirs = _get_unique_config_values(('LDLIBRARYDIR', 'srcdir', 'BINDIR', 'DESTLIB', 'DESTSHARED',
                                             'BINLIBDEST', 'LIBDEST', 'LIBDIR', 'MACHDESTLIB',))

    search_dirs.append(sys.prefix)

    if platform.system() is 'Windows':
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
        config_file = os.environ.get('JPY_PY_CONFIG', None)
        if config_file:
            # 3. Try 'JPY_PY_CONFIG' environment variable, if any
            return _read_config(config_file)

    return None


def preload_jvm_dll(jvm_dll_file=None, java_home_dir=None):
    if not jvm_dll_file:
        jvm_dll_file = find_jvm_dll_file(java_home_dir=java_home_dir, fail=True)
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
        jvm_cp = getattr(config, 'JPY_JVM_CLASSPATH', None)

    if not jvm_maxmem:
        jvm_maxmem = getattr(config, 'JPY_JVM_MAXMEM', None)

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

    if not java_home:
        java_home = os.environ.get('JPY_JAVA_HOME', None)
    if not jvm_dll:
        jvm_dll = getattr(config, 'JPY_JVM_DLL', None)

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
        jpy.create_jvm(options=jvm_options)
    else:
        jvm_options = None

    # print('jvm_dll =', jvm_dll)
    # print('jvm_options =', jvm_options)
    return (cdll, jvm_options, )


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


if __name__ == '__main__':
    import argparse
    import datetime

    comment = 'Created by ' + sys.argv[0] + '  on ' + str(datetime.datetime.now())

    parser = argparse.ArgumentParser(description='Generate configuration files for the jpy Python API (jpyconfig.py)\n'
                                                 'and the jpy Java API (jpyconfig.properties).')
    parser.add_argument("-o", "--out", action='store', default='.',
                        help="Output directory for the configuration files.")
    parser.add_argument("--java_home", action='store', default=None, help="Java home directory.")
    parser.add_argument("--jvm_dll", action='store', default=None, help="Java shared library location.")
    parser.add_argument("-f", "--force", action='store_true', default=False, help="Force output files to be overwritten.")
    args = parser.parse_args()

    java_api_properties_file = os.path.join(args.out, 'jpyconfig.properties')
    if not args.force and os.path.exists(java_api_properties_file):
        print("error: file exists: " + java_api_properties_file + " (use -f to force overwrite)")
        exit(1)

    python_api_config_file = os.path.join(args.out, 'jpyconfig.py')
    if not args.force and os.path.exists(python_api_config_file):
        print("error: file exists: " + python_api_config_file + " (use -f to force overwrite)")
        exit(1)

    java_api_properties = _get_java_api_properties(fail=True)

    jvm_dll = args.jvm_dll
    if not jvm_dll:
        jvm_dll = find_jvm_dll_file(java_home_dir=args.java_home)
    if not jvm_dll:
        print("error: can't determine any JVM shared library")
        exit(1)

    with open(python_api_config_file, 'w') as f:
        f.write('# ' + comment + '\"\n')
        if args.java_home:
            f.write('java_home = \"' + args.java_home.replace('\\', '\\\\') + '\"\n')
        f.write('jvm_dll = \"' + jvm_dll.replace('\\', '\\\\') + '\"\n')
        f.write('jvm_maxmem = None\n')
        f.write('jvm_classpath = []\n')
        f.write('jvm_properties = {}\n')
        f.write('jvm_options = []\n')
    print('Written jpy Python configuration to %s:' % (python_api_config_file,))
    print('  jvm_dll = %s' % (jvm_dll,))

    java_api_properties.store(java_api_properties_file, comments=[comment])
    print('Written jpy Java configuration to %s:' % (java_api_properties_file,))
    for key in java_api_properties.keys:
        print('  %s = %s' % (key, java_api_properties.values[key],))
