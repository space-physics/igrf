#!/usr/bin/env python
import setuptools  # noqa: F401
from numpy.distutils.core import setup, Extension
import os

f2py_options = ['--quiet']

if os.name == 'nt':
    f2py_options.append('--fcompiler=mingw32')

igrf12 = Extension(name='igrf12fort',
                   sources=['src/igrf12.f'],
                   f2py_options=f2py_options,
                   extra_f77_compile_args=['-w'])

igrf11 = Extension(name='igrf11fort',
                   sources=['src/igrf11.f'],
                   f2py_options=f2py_options,
                   extra_f77_compile_args=['-w'])

setup(ext_modules=[igrf12, igrf11])
