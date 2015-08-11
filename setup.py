#!/usr/bin/env python3
from numpy.distutils.core import setup,Extension

with open('README.rst') as f:
	long_description = f.read()

#%% install
setup(name='igrf12py',
      version='0.1',
	  description='Python wrapper for IGRF12 geomagnetic model',
	  long_description=long_description,
	  author='Michael Hirsch',
	  author_email='hirsch617@gmail.com',
	  url='https://github.com/scienceopen/igrf12py',
	  install_requires=['gridauora',
                     'numpy','six','pytz','dateutil'],
    dependency_links = ['https://github.com/scienceopen/gridaurora/tarball/master#egg=gridaurora'],
    packages=['igrf12py'],
    ext_modules=[Extension(name='igrf12',sources=['fortrancode/igrf12.f'],
                    f2py_options=['quiet'])]
	  )
