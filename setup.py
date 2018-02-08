from setuptools import setup, Extension
import sys

if sys.version_info >= (3, 0):
    module1 = [Extension('_cdifflib',
                        sources=['_cdifflib3.c'])]
else:
    module1 = [Extension('_cdifflib',
                        sources=['_cdifflib.c'])]

setup(name='cdifflib',
      version='1.1.0',
      description='C implementation of parts of difflib',
      ext_modules=module1,
      py_modules=['cdifflib'],
      test_suite='tests',

      author='Matthew Duggan',
      author_email='mgithub@guarana.org',
      license='BSD',
      url="https://github.com/mduggan/cdifflib",
      keywords="difflib c diff",

      classifiers=[
          'Development Status :: 4 - Beta',
          'Environment :: Console',
          'Intended Audience :: Developers',
          'License :: OSI Approved :: BSD License',
          'Operating System :: MacOS :: MacOS X',
          'Operating System :: Microsoft :: Windows',
          'Operating System :: POSIX',
          'Programming Language :: Python',
          'Topic :: Software Development',
          'Topic :: Text Processing :: General',
      ],
      )
