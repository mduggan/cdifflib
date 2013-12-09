from setuptools import setup, Extension

module1 = Extension('_cdifflib',
                    sources=['_cdifflib.c'])

setup(name='CDiffLib',
      version='1.0',
      description='C implementation of parts of difflib',
      ext_modules=[module1],
      py_modules=['cdifflib'],
      test_suite='tests',

      classifiers=[
          'Development Status :: 4 - Beta',
          'Environment :: Console',
          'Intended Audience :: Developers',
          'License :: Proprietary',
          'Operating System :: MacOS :: MacOS X',
          'Operating System :: Microsoft :: Windows',
          'Operating System :: POSIX',
          'Programming Language :: Python',
          'Topic :: Software Development',
          'Topic :: Text Processing :: General',
      ],
      )
