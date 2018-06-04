.. image:: https://travis-ci.org/scivision/pyigrf12.svg?branch=master
    :target: https://travis-ci.org/scivision/pyigrf12

.. image:: https://coveralls.io/repos/github/scivision/pyigrf12/badge.svg?branch=master
    :target: https://coveralls.io/github/scivision/pyigrf12?branch=master

.. image:: https://img.shields.io/pypi/pyversions/pyigrf12.svg
  :target: https://pypi.python.org/pypi/pyigrf12
  :alt: Python versions (PyPI)

.. image::  https://img.shields.io/pypi/format/pyigrf12.svg
  :target: https://pypi.python.org/pypi/pyigrf12
  :alt: Distribution format (PyPI)
  
.. image:: http://pepy.tech/badge/pyigrf12
   :target: http://pepy.tech/project/pyigrf12
   :alt: PyPi Download stats

========
igrf12py
========
International Geomagnetic Reference Field IGRF12 and IGRF11...in simple, object-oriented Python >= 3.6.

:Author Python API: Michael Hirsch, Ph.D.

.. image:: tests/incldecl.png

.. image:: tests/vectors.png

Install
=======
To get the cutting-edge development version, ``git clone`` and then::

    python -m pip install -e .

Otherwise, for the latest release from PyPi::

    python -m pip install pyigrf12

You can test the install with::

    pytest

Run Example
===========
::

    python RunIGRF.py

Reference
=========
If you only want the plain Fortran program, you can do:

.. code:: bash

    cd bin
    cmake ../fortran
    make
    ./testigrf


References
-----------

* `IGRF12 Fortran code <http://www.ngdc.noaa.gov/IAGA/vmod/igrf12.f>`_
* `IGRF11 Fortran code <http://www.ngdc.noaa.gov/IAGA/vmod/igrf11.f>`_

* WMM2015 `inclination map <https://www.ngdc.noaa.gov/geomag/WMM/data/WMM2015/WMM2015_I_MERC.pdf>`_
* WMM2015 `declination map <https://www.ngdc.noaa.gov/geomag/WMM/data/WMM2015/WMM2015_D_MERC.pdf>`_
