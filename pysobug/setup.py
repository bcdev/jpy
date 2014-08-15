#!/usr/bin/env python3

from distutils.core import setup
from distutils.extension import Extension

ext = Extension('mypymod',
                sources=['mypymod.c'],
                #libraries=['python3.4m'],
)

setup(name='mypymod', ext_modules=[ext])



