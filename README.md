[![image](https://travis-ci.org/scivision/pyigrf12.svg?branch=master)](https://travis-ci.org/scivision/pyigrf12)
[![image](https://coveralls.io/repos/github/scivision/pyigrf12/badge.svg?branch=master)](https://coveralls.io/github/scivision/pyigrf12?branch=master)
[![Python versions (PyPI)](https://img.shields.io/pypi/pyversions/pyigrf12.svg)](https://pypi.python.org/pypi/pyigrf12)
[![Distribution format (PyPI)](https://img.shields.io/pypi/format/pyigrf12.svg)](https://pypi.python.org/pypi/pyigrf12)
[![PyPi Download stats](http://pepy.tech/badge/pyigrf12)](http://pepy.tech/project/pyigrf12)

# IGRF 2012 in Python

International Geomagnetic Reference Field IGRF12 and IGRF11...in simple, object-oriented Python >= 3.6.

![image](tests/incldecl.png)

![image](tests/vectors.png)

## Install

To get the development version, `git clone` and then:

    python -m pip install -e .

Otherwise, for the latest release from PyPi:

    python -m pip install pyigrf12

Optionally, test the install with:

    pytest

## Example

    python RunIGRF.py

## Reference

If you only want the plain Fortran program, you can do:

```sh
cd bin

cmake ../src
cmake --build .

./testigrf
```

### References

-   [IGRF12 Fortran code](http://www.ngdc.noaa.gov/IAGA/vmod/igrf12.f)
-   [IGRF11 Fortran code](http://www.ngdc.noaa.gov/IAGA/vmod/igrf11.f)
-   WMM2015 [inclination map](https://www.ngdc.noaa.gov/geomag/WMM/data/WMM2015/WMM2015_I_MERC.pdf)
-   WMM2015 [declination map](https://www.ngdc.noaa.gov/geomag/WMM/data/WMM2015/WMM2015_D_MERC.pdf)
