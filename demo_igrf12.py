#!/usr/bin/env python3
"""
NOTE: The performance of this demo has not been checked at all.
Please do basic sanity checks of output.
Quick demo of calling IGRF12 and IGRF11 using f2py3 from Python
Michael Hirsch
bostonmicrowave.com

"""
from __future__ import division
from numpy import  arange, empty, empty_like, meshgrid, atleast_1d,nditer
from matplotlib.pyplot import figure,show
#
import igrf12
import igrf11

def testigrf12(isv,year,itype,alt,colat,elon):

    x = empty(colat.size);  y = empty_like(x); z = empty_like(x); f=empty_like(x)
    for i,(clt,eln) in enumerate(nditer((colat,elon))):
        x[i],y[i],z[i],f[i] = igrf12.igrf12syn(isv, year, itype, alt, clt, eln)

    return x.reshape(colat.shape), y.reshape(colat.shape), z.reshape(colat.shape),f.reshape(colat.shape)

def testigrf11(isv,year,itype,alt,colat,elon):

    x = empty(colat.size);  y = empty_like(x); z = empty_like(x); f=empty_like(x)
    for i,(clt,eln) in enumerate(nditer((colat,elon))):
        x[i],y[i],z[i],f[i] = igrf11.igrf11syn(isv, year, itype, alt, clt, eln)

    return x.reshape(colat.shape), y.reshape(colat.shape), z.reshape(colat.shape),f.reshape(colat.shape)


def plotigrf(x,y,z,f,glat,glon,year,isv,mdl):
    for i,j in zip((x,y,z),('x','y','z')):
        fg = figure()
        ax = fg.gca()
        hi = ax.imshow(i,extent=(glon[0,0],glon[-1,0],glat[0,0],glat[0,-1]))
        fg.colorbar(hi)
        ax.set_xlabel('latitude (deg)')
        ax.set_ylabel('longitude (deg)')
        ax.set_title('IGRF{:s} $B_{:s}$-field on {:.2f}'.format(mdl,j,year))

    if isv==0:
        fg = figure()
        ax = fg.gca()
        hi = ax.imshow(f,extent=(glon[0,0],glon[-1,0],glat[0,0],glat[0,-1]))
        fg.colorbar(hi)
        ax.set_xlabel('latitude (deg)')
        ax.set_ylabel('longitude (deg)')
        ax.set_title('IGRF{:s} $B$-field: total intensity [nT] on {:.2f}'.format(mdl,year))

def plotdiff1112(x,x11,y,y11,z,z11,f,f11,glat,glon,year,isv):
    for i,j,k in zip((x,y,z),(x11,y11,z11),('x','y','z')):
        fg = figure()
        ax = fg.gca()
        hi = ax.imshow(i-j,extent=(glon[0,0],glon[-1,0],glat[0,0],glat[0,-1]))
        fg.colorbar(hi)
        ax.set_xlabel('latitude (deg)')
        ax.set_ylabel('longitude (deg)')
        ax.set_title('IGRF12-IGRF11 $B_{:s}$-field comparison on {:.2f}'.format(k,year))

    if isv==0:
        fg = figure()
        ax = fg.gca()
        hi = ax.imshow(f-f11,extent=(glon[0,0],glon[-1,0],glat[0,0],glat[0,-1]))
        fg.colorbar(hi)
        ax.set_xlabel('latitude (deg)')
        ax.set_ylabel('longitude (deg)')
        ax.set_title('IGRF12-IGRF11 $B$-field: comparison total intensity [nT] on {:.2f}'.format(year))


if __name__ == '__main__':
    from argparse import ArgumentParser
    p = ArgumentParser(description='calls HWM93 from Python, a basic demo')
    p.add_argument('year',help='year.y Common Era year+fractional part of year',type=float,nargs='?',default=2015.4)
    p.add_argument('--isv',help='0: main field. 1: secular variation',type=int,default=0)
    p.add_argument('--itype',help='1: geodetic. 2: geocentric',type=int,default=1)
    p.add_argument('altkm',help='(km) above sea level if itype=1, (km) from center of Earth if itype=2',type=float,nargs='?',default=0)
    p.add_argument('-c','--latlon',help='geodetic latitude, longitude (deg)',type=float,nargs=2,default=(None,None))
    p = p.parse_args()

    # do world-wide grid if no user input
    if p.altkm is None or p.latlon[0] is None:
        lat = arange(-90,90+5,5)
        lon = arange(-180,180+10,10)
        glat,glon = meshgrid(lat,lon)
    elif p.altkm is not None and p.latlon[0] is not None:
        glat,glon = p.latlon
    else:
        exit('please input all 3 of lat,lon,alt or none of them')

    #atleast_1d for iteration later
    colat = 90-atleast_1d(glat)
    elon = (360 + atleast_1d(glon)) % 360

    x,y,z,f = testigrf12(p.isv, p.year, p.itype, p.altkm, colat,elon)
    x11,y11,z11,f11 = testigrf11(p.isv, p.year, p.itype, p.altkm, colat,elon)

    if colat.ndim==2:
        plotigrf(x,y,z,f,glat,glon,p.year,p.isv,'12')
        #plotigrf(x,y,z,f,glat,glon,p.year,p.isv,'11')

        plotdiff1112(x,x11,y,y11,z,z11,f,f11,glat,glon,p.year,p.isv)
    else:
        print('x y z f')
        print(x,y,z,f)

    show()
