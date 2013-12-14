__author__ = 'Norman'

import sys
import importlib.abc


def new_module(name):
    """
    Create a new module.
    """
    print('creating module "' + name + '"')
    return type(sys)(name)


class File:
    def __init__(self, path):
        self.path = path

class String:
    def __init__(self, str):
        self.str = str


class JavaPackageImporter(importlib.abc.MetaPathFinder):

    def find_module(self, fullname, path):
        #print('find_module(fullname="' + str(fullname) + '", path="' + str(path) + '")')
        if fullname == 'java' or fullname.startswith('java.'):
            return self
        return None

    def load_module(self, fullname):
        print('load_module(fullname="' + str(fullname) + '")')
        is_reload = fullname in sys.modules
        if is_reload:
            module = sys.modules[fullname]
            print('module found!')
            # Now, check what to do next...?
        else:
            module = new_module(fullname)
            sys.modules[fullname] = module
            module.__loader__ = self
            module.__path__ = None
            dot_pos = fullname.rfind('.')
            module.__package__ = fullname[:dot_pos] if dot_pos > 0 else ''
            # Now, call into C-lib and load Java package or class...
            print('new module: ' + module.__name__)
            if module.__name__ == 'java.io':
                module.File = File
            if module.__name__ == 'java.lang':
                module.String = String
        return module



sys.meta_path = [JavaPackageImporter()] + sys.meta_path

#import java.io
#import java.lang
#import java.util
#import numpy
#import java.wraaaw
from java.util import String
from java.io import File

s = java.lang.String('Hello')
print(f)

f = java.io.File('x')
print(f)


#import os.path
#
#print(os.__name__)
#print(os.__package__)
#print(os.__file__)
#
#print(dir(java.io))
#
#
#print(os.__name__)
#print(os.__package__)
#print(os.__file__)
#print(list(os.__path__))
#print(os.path.__name__)
#print(os.path.__package__)
#print(os.path.__file__)
#print(list(os.path.__path__))
#
#print('java.io:', dir(java.io))

