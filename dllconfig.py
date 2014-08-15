import sysconfig
import os.path
import platform
#import pprint

#pprint.pprint(sysconfig.get_config_vars())

def get_config_values(names):
    values = []
    for name in names:
        value = sysconfig.get_config_var(name)
        if value and not value in values:
            values.append(value)
    return values


def find_jvm_shared_lib(lib_dirs):
    if platform.system() is 'Windows':
        libs = ['jvm.dll']
    else:
        libs = ['libjvm.so']
    jvm_shared_lib = None
    for lib_dir in lib_dirs:
        for lib in libs:
            path = os.path.join(lib_dir, lib)
            if os.path.exists(path):
                #print('Found:', path)
                if not jvm_shared_lib:
                    jvm_shared_lib = path
    return jvm_shared_lib


def find_python_shared_lib():

    libs = get_config_values(['LDLIBRARY', 'INSTSONAME', 'PY3LIBRARY', 'DLLLIBRARY'])
    lib_dirs = get_config_values(['LDLIBRARYDIR', 'srcdir', 'BINDIR', 'DESTLIB', 'DESTSHARED', 'BINLIBDEST',
                                  'LIBDEST', 'LIBDIR', 'MACHDESTLIB'])

    if platform.system() is 'Windows':
        libs.append('python3.dll')
        libs.append('python.dll')
        lib_dirs_extra = [os.path.join(lib, 'DLLs') for lib in lib_dirs]
        lib_dirs = lib_dirs_extra + lib_dirs
    else:
        multiarchsubdir = sysconfig.get_config_var('multiarchsubdir')
        if multiarchsubdir:
            if multiarchsubdir.startswith('/'):
                multiarchsubdir = multiarchsubdir[1:]
            lib_dirs_extra = [os.path.join(lib, multiarchsubdir) for lib in lib_dirs]
            lib_dirs = lib_dirs_extra + lib_dirs

    #pprint.pprint(libs)
    #pprint.pprint(lib_dirs)

    python_shared_lib = None

    for lib_dir in lib_dirs:
        for lib in libs:
            path = os.path.join(lib_dir, lib)
            if os.path.exists(path):
                #print('Found:', path)
                if not python_shared_lib:
                    python_shared_lib = path

    return python_shared_lib