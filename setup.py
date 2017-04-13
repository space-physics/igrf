#!/usr/bin/env python
import setuptools
from numpy.distutils.core import setup,Extension

req = ['nose','python-dateutil','pytz','numpy','matplotlib','seaborn',
        'gridaurora']

#%% install
setup(name='igrf12py',
      packages=['igrf12py'],
      version='1.0',
      author='Michael Hirsch, Ph.D.',
      url='https://github.com/scivision/igrf12py',
      description='IGRF 2012 model accessed from Python',
	  install_requires=req,
      ext_modules=[Extension(name='igrf12',
                           sources=['fortran/igrf12.f'],
                           f2py_options=['--quiet'])]
	  )
