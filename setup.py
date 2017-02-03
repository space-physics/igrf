#!/usr/bin/env python
import setuptools
try:
    import conda.cli
    conda.cli.main('install','--file','requirements.txt')
except Exception as e:
    print(e)


from numpy.distutils.core import setup,Extension

#%% install
setup(name='igrf12py',
	  install_requires=['gridaurora','histutils'],
      dependency_links = ['https://github.com/scienceopen/gridaurora/tarball/master#egg=gridaurora',
                          'https://github.com/scienceopen/histutils/tarball/master#egg=histutils'],
      packages=['igrf12py'],
      ext_modules=[Extension(name='igrf12',
                           sources=['fortrancode/igrf12.f'],
                           f2py_options=['--quiet'])]
	  )
