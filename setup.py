from distutils.core import setup, Extension

module1 = Extension('_cdifflib',
                    sources=['_cdifflib.c'])

setup(name='CDiffLib',
      version='1.0',
      description='C implementation of parts of difflib',
      ext_modules=[module1], py_modules=['cdifflib'])
