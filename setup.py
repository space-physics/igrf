#!/usr/bin/env python3
import setuptools #by importing this first, you get the ability to use ext_modules and install_requires, best of both worlds
from numpy.distutils.core import setup,Extension

with open('README.rst','r') as f:
	long_description = f.read()

#%% install
setup(name='igrf12py',
      version='0.1',
	  description='Python wrapper for IGRF12 geomagnetic model',
	  long_description=long_description,
	  author='Michael Hirsch',
	  author_email='hirsch617@gmail.com',
	  url='https://github.com/scienceopen/igrf12py',
	  install_requires=['gridaurora','histutils',
                     'numpy','six','pytz','python-dateutil'],
      dependency_links = ['https://github.com/scienceopen/gridaurora/tarball/master#egg=gridaurora',
                          'https://github.com/scienceopen/histutils/tarball/master#egg=histutils'],
      packages=['igrf12py'],
      ext_modules=[Extension(name='igrf12',
                           sources=['fortrancode/igrf12.f'],
                           f2py_options=['quiet'])]
	  )
