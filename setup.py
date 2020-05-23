#!/usr/bin/env python
import setuptools  # noqa: F401
from numpy.distutils.core import setup, Extension

igrf13 = Extension(name="igrf13fort", sources=["src/igrf13.f"], f2py_options=["--quiet"], extra_f77_compile_args=["-w"])

igrf12 = Extension(name="igrf12fort", sources=["src/igrf12.f"], f2py_options=["--quiet"], extra_f77_compile_args=["-w"])

igrf11 = Extension(name="igrf11fort", sources=["src/igrf11.f"], f2py_options=["--quiet"], extra_f77_compile_args=["-w"])

setup(ext_modules=[igrf13, igrf12, igrf11])
