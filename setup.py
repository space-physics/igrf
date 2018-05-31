#!/usr/bin/env python
from setuptools import find_packages
from numpy.distutils.core import setup, Extension
from typing import List

install_requires: List[str] = ['xarray', 'numpy', 'sciencedates']
tests_require = ['pytest', 'nose', 'coveralls']
# %% install
setup(name='pyigrf12',
      packages=find_packages(),
      version='1.3.2',
      author='Michael Hirsch, Ph.D.',
      url='https://github.com/scivision/pyigrf12',
      description='IGRF12 model with simple object-oriented Python interface',
      long_description=open('README.rst').read(),
      install_requires=install_requires,
      ext_modules=[Extension(name='igrf12',
                             sources=['fortran/igrf12.f'],
                             f2py_options=['--quiet'],
                             extra_f77_compile_args=['-w'])],
      classifiers=[
          'Development Status :: 5 - Production/Stable',
          'Intended Audience :: Science/Research',
          'License :: OSI Approved :: MIT License',
          'Programming Language :: Python :: 3.6',
          'Programming Language :: Python :: 3.7',
          'Topic :: Scientific/Engineering :: Atmospheric Science',
          ],
      extras_require={'plot': ['matplotlib'],
                      'tests': tests_require},
      python_requires='>=3.6',
      tests_require=tests_require,
      )
