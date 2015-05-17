[![Code Climate](https://codeclimate.com/github/scienceopen/igrf12py/badges/gpa.svg)](https://codeclimate.com/github/scienceopen/igrf12py)

igrf12py
========
International Geomagnetic Reference Field IGRF12 and IGRF11...in Python!

Installation
------------
```
git clone --depth 1 --recursive https://github.com/scienceopen/igrf12py
pip install -r requirements.txt
f2py -m igrf12 -c igrf12.f 
f2py -m igrf11 -c igrf11.f 
```

Run Example
-----------
```
python demo_igrf12.py
```


Reference
-----------
http://www.ngdc.noaa.gov/IAGA/vmod/igrf12.f

http://www.ngdc.noaa.gov/IAGA/vmod/igrf11.f
