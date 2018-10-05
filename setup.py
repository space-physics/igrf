#!/usr/bin/env python
import setuptools  # noqa: F401
from numpy.distutils.core import setup, Extension
from pathlib import Path
import os


if os.name == 'nt':
    sfn = Path(__file__).parent / 'setup.cfg'
    stxt = sfn.read_text()
    if '[build_ext]' not in stxt:
        with sfn.open('a') as f:
            f.write("[build_ext]\ncompiler = mingw32")


igrf12 = Extension(name='igrf12fort',
                   sources=['src/igrf12.f'],
                   extra_f77_compile_args=['-w'])

igrf11 = Extension(name='igrf11fort',
                   sources=['src/igrf11.f'],
                   extra_f77_compile_args=['-w'])

setup(ext_modules=[igrf12, igrf11])
