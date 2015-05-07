igrf12py
International Geomagnetic Reference Field IGRF12 and IGRF11...in Python!

All credit to original authors, I slightly modified the Fortran 77 
code so it could compile in a modern compiler. 

```
f2py -m igrf12 -c igrf12.f 
f2py -m igrf11 -c igrf11.f 
```
plot by:
```
python demo_igrf12.py
```

Prereqs:
--------
``` pip install -r requirements.txt ```

#### Reference
http://www.ngdc.noaa.gov/IAGA/vmod/igrf12.f
http://www.ngdc.noaa.gov/IAGA/vmod/igrf11.f
