========
igrf12py
========
International Geomagnetic Reference Field IGRF12 and IGRF11...in Python!

:Author Python API: Michael Hirsch, Ph.D.

.. image:: .github.demoigrf.png

Installation
============
::

    python setup.py develop

Run Example
===========
::

    python demo_igrf12.py


Reference
=========
If you only want the plain Fortran program, you can do::

    cd bin
    cmake ../fortran
    make
    ./testigrf

References
-----------
http://www.ngdc.noaa.gov/IAGA/vmod/igrf12.f

http://www.ngdc.noaa.gov/IAGA/vmod/igrf11.f
