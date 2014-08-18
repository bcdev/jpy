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


PYTHON_LIB_NAME = _get_python_lib_name()
JVM_LIB_NAME = 'jvm'
IS64BIT = sys.maxsize > 2 ** 32


def _get_unique_config_values(names):
    values = []
    for name in names:
        value = sysconfig.get_config_var(name)
        if value and not value in values:
            values.append(value)
    return values


def preload_jvm_dll(lib_path=None, java_home_dir=None):
    if not lib_path:
        lib_path = find_jvm_dll_path(java_home_dir)

    if not lib_path:
        raise ValueError('JVM shared library not found')

    return ctypes.CDLL(lib_path)


def _add_paths_if_exists(path_list, *paths):
    for path in paths:
        if os.path.exists(path) and not path in path_list:
            path_list.append(path)
    return path_list


def get_jpy_config_file_path():
    user_home = os.path.expanduser('~')
    return os.path.join(user_home, '.jpy')


def find_jvm_dll_path(java_home_dir=None):
    if java_home_dir:
        jvm_dll_path = _find_jvm_dll_path(java_home_dir)
        if jvm_dll_path:
            return jvm_dll_path

    jpy_config_file_path = get_jpy_config_file_path()
    if os.path.exists(jpy_config_file_path):
        jpy_config = Properties()
        jpy_config.load(jpy_config_file_path)
        jvm_dll_path = jpy_config.get_property('jvm.lib')
        if jvm_dll_path:
            return jvm_dll_path

    for name in ('JPY_JAVA_HOME', 'JAVA_HOME', 'JDK_HOME', 'JRE_HOME', 'JAVA_JRE'):
        java_home_dir = os.environ.get(name, None)
        if java_home_dir:
            jvm_dll_path = _find_jvm_dll_path(java_home_dir)
            if jvm_dll_path:
                return jvm_dll_path

    return ctypes.util.find_library(JVM_LIB_NAME)


def _get_jvm_lib_dirs(java_home_dir):
    arch = 'amd64' if IS64BIT else 'i386'
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


def _find_jvm_dll_path(java_home_dir):
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


def _find_file(search_dirs, *filenames):
    for dir in search_dirs:
        dir = os.path.normpath(dir)
        for filename in filenames:
            path = os.path.join(dir, filename)
            if os.path.exists(path):
                return path
    return None


def find_python_dll_path():
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

    return python_dll_path


class Properties:
    def __init__(self):
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

