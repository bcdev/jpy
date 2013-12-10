__author__ = 'Norman'

import sys
import importlib.abc


def new_module(name):
    """
    Create a new module.
    """
    return type(sys)(name)


class File:
    def __init__(self, path):
        self.path = path

class Float:
    def __init__(self, num):
        self.num = num


class JavaPackageImporter(importlib.abc.MetaPathFinder):

    def find_module(self, fullname, path):
        print('find_module(fullname="' + str(fullname) + '", path="' + str(path) + '")')
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
            module.File = File
            module.Float = Float
        return module



sys.meta_path = [JavaPackageImporter()] + sys.meta_path

import java.io
import java.lang
#import java.util
import numpy
import java.wraaaw
from java.util import File as F

f = java.lang.Float(8)
f = java.lang.File('x')
f = F('y')


import os.path

print(os.__name__)
print(os.__package__)
print(os.__file__)

print(dir(java.io))


print(os.__name__)
print(os.__package__)
print(os.__file__)
#print(list(os.__path__))
print(os.path.__name__)
print(os.path.__package__)
print(os.path.__file__)
#print(list(os.path.__path__))

print('java.io:', dir(java.io))

