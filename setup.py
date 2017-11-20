#!/usr/bin/env python
req = ['nose','numpy',
       'sciencedates']

# %%
from setuptools import find_packages
from numpy.distutils.core import setup,Extension
#%% install
setup(name='pyigrf12',
      packages=find_packages(),
      version='1.1',
      author='Michael Hirsch, Ph.D.',
      url='https://github.com/scivision/pyigrf12',
      description='IGRF12 model accessed from Python',
	  install_requires=req,
      ext_modules=[Extension(name='igrf12',
                           sources=['fortran/igrf12.f'],
                           f2py_options=['--quiet'])],
      classifiers=[
          'Intended Audience :: Science/Research',
          'Development Status :: 5 - Production/Stable',
          'License :: OSI Approved :: MIT License',
          'Topic :: Scientific/Engineering :: Atmospheric Science',
          'Programming Language :: Python :: 3',
          ],
      extras_requires={'plot':['gridaurora','matplotlib','seaborn']},
      python_requires='>=3.6',
	  )
