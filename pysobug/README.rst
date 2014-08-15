#############################
Python shared library problem
#############################

Problem: Native Python C-extensions can't find Python symbols if Python interpreter is embedded into a shared library.

mypydl.c - Source for a shared libary (mypydl.so) that embeds a Python interpreter
mypy.c - Source for a main program (mypy) that dynamically loads the shared library mypydl.so
mypymod.c - Native Python extension module (mypymod) that uses the Python C-API

make.sh - builds mypydl.so, mypy and the mypymod Python extension module.

The main program mypy interprets all given arguments as Python code and executes it.
Here is how to reproduce the problem:

$ bash ./make.sh
$ ./mypy "print('Hello')"
mypy: executing [print('Hello')]
Hello
mypy: status 0


$ ./mypy "import mypymod"
mypy: executing [import mypymod]
Traceback (most recent call last):
  File "<string>", line 1, in <module>
ImportError: /home/norman/.local/lib/python3.4/site-packages/mypymod.cpython-34m.so: undefined symbol: Py_BuildValue
...
mypy: status -1


$ ./mypy "import numpy"
mypy: executing [import numpy]
...
ImportError: /home/norman/.local/lib/python3.4/site-packages/numpy/core/multiarray.cpython-34m.so: undefined symbol: PyExc_SystemError
mypy: status -1
