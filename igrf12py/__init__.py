"""
NOTE: The performance of this demo has not been checked at all.
Please do basic sanity checks of output.
Quick demo of calling IGRF12 and IGRF11 using f2py3 from Python
Michael Hirsch
"""
from numpy import  empty, empty_like, atleast_1d,nditer
#
from sciencedates import datetime2yeardec
#
import igrf12
#import igrf11

def runigrf12(dtime,isv,itype,alt,glat,glon):

    yeardec = datetime2yeardec(dtime)
    colat,elon = latlon2colat(glat,glon)

    x = empty(colat.size);  y = empty_like(x); z = empty_like(x); f=empty_like(x)
    for i,(clt,eln) in enumerate(nditer((colat,elon))):
        x[i],y[i],z[i],f[i] = igrf12.igrf12syn(isv, yeardec, itype, alt, clt, eln)

    return x.reshape(colat.shape), y.reshape(colat.shape), z.reshape(colat.shape),f.reshape(colat.shape), yeardec

def runigrf11(dtime,isv,itype,alt,glat,glon):

    yeardec = datetime2yeardec(dtime)
    colat,elon = latlon2colat(glat,glon)

    x = empty(colat.size);  y = empty_like(x); z = empty_like(x); f=empty_like(x)
    for i,(clt,eln) in enumerate(nditer((colat,elon))):
        x[i],y[i],z[i],f[i] = igrf11.igrf11syn(isv, yeardec, itype, alt, clt, eln)

    return x.reshape(colat.shape), y.reshape(colat.shape), z.reshape(colat.shape),f.reshape(colat.shape)

def latlon2colat(glat,glon):
    #atleast_1d for iteration later
    colat = 90-atleast_1d(glat)
    elon = (360 + atleast_1d(glon)) % 360
    return colat,elon
