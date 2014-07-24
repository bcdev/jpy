import os
import sys
import sysconfig
from datetime import datetime

class Properties:

    def __init__(self):
        self.keys = []
        self.values = {}


    def setproperty(self, key, value):
        if value:
            if not key in self.keys:
                self.keys.append(key)
            self.values[key] = value
        else:
            if key in self.keys:
                self.keys.remove(key)
                self.values.pop(key)


    def getproperty(self, key):
        return self.values[key] if key in self.values else None


    def store(self, path, comments=()):
        with open(path, 'w') as f:
            for comment in comments:
                f.write('# ' + str(comment).replace('\\', '\\\\') + '\n')
            for key in self.keys:
                value = self.getproperty(key)
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
                        self.setproperty(tokens[0].strip(), tokens[1].strip().replace('\\\\', '\\'))
                    else:
                        raise ValueError('illegal Java properties format ' + line)



file = os.path.join('.', 'sysconfig.properties')

properties = Properties()
update = False
if os.path.exists(file):
    properties.load(file)
    update = True


IS64 = sys.maxsize > 2 ** 32

property_prefix = 'python' + str(sys.version_info.major) + str(sys.version_info.minor) + '_' + ('64' if IS64 else '32')

config_vars = sysconfig.get_config_vars()
for key in config_vars:
    properties.setproperty(property_prefix + '.config_vars.' + key, config_vars[key])

paths = sysconfig.get_paths()
for key in paths:
    properties.setproperty(property_prefix + '.paths.' + key, paths[key])

properties.setproperty(property_prefix + '.platform', sysconfig.get_platform())

comment = ('Updated' if update else 'Created') + ' by ' + __file__ + ' on ' + str(datetime.now())
properties.store(file, comments=['', comment, ''])




