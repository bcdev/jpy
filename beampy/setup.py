#!/usr/bin/env python3

from distutils.core import setup

setup(name='beampy',
      version='0.3',
      description='BEAM Python API',
      author='Norman Fomferra, Brockmann Consult GmbH',
      py_modules=['beampy'],
      )

import sys
if sys.argv[1] == 'install':
    import os
    os.system(sys.executable + ' beampy_test.py')
