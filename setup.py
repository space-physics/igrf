#!/usr/bin/env python
import setuptools
try:
    import conda.cli
    conda.cli.main('install','--file','requirements.txt')
except Exception as e:
    print(e)
    import pip
    pip.main(['install','-r','requirements.txt'])


from numpy.distutils.core import setup,Extension

#%% install
setup(name='igrf12py',
      packages=['igrf12py'],
      author='Michael Hirsch, Ph.D.',
      url='https://github.com/scienceopen/igrf12py',
      description='IGRF 2012 model accessed from Python',
	  install_requires=['gridaurora'],
      dependency_links = [
      'https://github.com/scienceopen/gridaurora/tarball/master#egg=gridaurora',
                        ],

      ext_modules=[Extension(name='igrf12',
                           sources=['fortrancode/igrf12.f'],
                           f2py_options=['--quiet'])]
	  )
