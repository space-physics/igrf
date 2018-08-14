#!/usr/bin/env python
import setuptools  # noqa: F401
from numpy.distutils.core import setup, Extension

igrf12 = Extension(name='igrf12fort',
                   sources=['src/igrf12.f'],
                   extra_f77_compile_args=['-w'])

igrf11 = Extension(name='igrf11fort',
                   sources=['src/igrf11.f'],
                   extra_f77_compile_args=['-w'])

setup(ext_modules=[igrf12, igrf11])
