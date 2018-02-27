#!/usr/bin/env python
install_requires = ['numpy','sciencedates']
tests_require=['nose','coveralls']

# %%
from setuptools import find_packages
from numpy.distutils.core import setup,Extension
#%% install
setup(name='pyigrf12',
      packages=find_packages(),
      version='1.2.0',
      author='Michael Hirsch, Ph.D.',
      url='https://github.com/scivision/pyigrf12',
      description='IGRF12 model accessed from Python',
	  install_requires=install_requires,
      ext_modules=[Extension(name='igrf12',
                           sources=['fortran/igrf12.f'],
                           f2py_options=['--quiet'],
                           extra_f77_compile_args=['-w'])],
      classifiers=[
          'Intended Audience :: Science/Research',
          'Development Status :: 5 - Production/Stable',
          'License :: OSI Approved :: MIT License',
          'Topic :: Scientific/Engineering :: Atmospheric Science',
          'Programming Language :: Python :: 3',
          ],
      extras_require={'plot':['matplotlib'],
                       'tests':tests_require,},
      python_requires='>=3.6',
      tests_require=tests_require,
	  )
