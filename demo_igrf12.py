#!/usr/bin/env python3
"""
NOTE: The performance of this demo has not been checked at all.
Please do basic sanity checks of output.
Quick demo of calling IGRF12 and IGRF11 using f2py3 from Python
Michael Hirsch
bostonmicrowave.com

"""
from __future__ import division
from numpy import  empty, empty_like, atleast_1d,nditer
from matplotlib.pyplot import figure,show,subplots
from matplotlib.ticker import ScalarFormatter
import sys
sys.path.append('../msise-00') #git clone https://github.com/scienceopen/msise-00.git
from demo_msis import latlonworldgrid
from fortrandates import datetime2yeardec
#
try:
    import igrf12
    import igrf11
except ImportError as e:
    exit('you must compile using f2py. Please see README.md. ' + str(e))

sfmt = ScalarFormatter(useMathText=True) #for 10^3 instead of 1e3
sfmt.set_powerlimits((-2, 2))
sfmt.set_scientific(True)
sfmt.set_useOffset(False)

def testigrf12(dtime,isv,itype,alt,colat,elon):

    yeardec = datetime2yeardec(dtime)
    colat,elon = latlon2colat(glat,glon)

    x = empty(colat.size);  y = empty_like(x); z = empty_like(x); f=empty_like(x)
    for i,(clt,eln) in enumerate(nditer((colat,elon))):
        x[i],y[i],z[i],f[i] = igrf12.igrf12syn(isv, yeardec, itype, alt, clt, eln)

    return x.reshape(colat.shape), y.reshape(colat.shape), z.reshape(colat.shape),f.reshape(colat.shape), yeardec

def testigrf11(dtime,isv,itype,alt,glat,glon):

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

def plotigrf(x,y,z,f,glat,glon,year,isv,mdl):
    fg,ax = subplots(2,2,sharex=True)
    ax = ax.ravel()
    for a,i,j in zip(ax,(x,y,z),('x','y','z')):
        hi = a.imshow(i,extent=(glon[0,0],glon[0,-1],glat[0,0],glat[-1,0]),
                      cmap='bwr',
                      vmin=-6e4,vmax=6e4) #symmetrix vmin,vmax centers white at zero for bwr cmap
        fg.colorbar(hi,ax=a,format=sfmt)
        a.set_title('IGRF{:s} $B_{:s}$-field on {:.3f}'.format(mdl,j,year))
    for a in ax[[0,2]]:
        a.set_ylabel('latitude (deg)')
    for a in ax[[2,3]]:
        a.set_xlabel('longitude (deg)')


    if isv==0:
        hi = a.imshow(f,extent=(glon[0,0],glon[0,-1],glat[0,0],glat[-1,0]))
        fg.colorbar(hi,ax=a,format=sfmt)
        a.set_title('IGRF{:s} $B$-field: total intensity [nT] on {:.2f}'.format(mdl,year))

def plotdiff1112(x,x11,y,y11,z,z11,f,f11,glat,glon,year,isv):
    for i,j,k in zip((x,y,z),(x11,y11,z11),('x','y','z')):
        fg = figure()
        ax = fg.gca()
        hi = ax.imshow(i-j,extent=(glon[0,0],glon[0,-1],glat[0,0],glat[-1,0]))
        fg.colorbar(hi,format=sfmt)
        ax.set_ylabel('latitude (deg)')
        ax.set_xlabel('longitude (deg)')
        ax.set_title('IGRF12-IGRF11 $B_{:s}$-field comparison on {:.2f}'.format(k,year))

    if isv==0:
        fg = figure()
        ax = fg.gca()
        hi = ax.imshow(f-f11,extent=(glon[0,0],glon[0,-1],glat[0,0],glat[-1,0]))
        fg.colorbar(hi)
        ax.set_xlabel('latitude (deg)')
        ax.set_ylabel('longitude (deg)')
        ax.set_title('IGRF12-IGRF11 $B$-field: comparison total intensity [nT] on {:.2f}'.format(year))


if __name__ == '__main__':
    from argparse import ArgumentParser
    p = ArgumentParser(description='calls HWM93 from Python, a basic demo')
    p.add_argument('simtime',help='yyyy-mm-ddTHH:MM:SSZ time of sim',type=str,nargs='?',default='')
    p.add_argument('--isv',help='0: main field. 1: secular variation',type=int,default=0)
    p.add_argument('--itype',help='1: geodetic. 2: geocentric',type=int,default=1)
    p.add_argument('-a','--altkm',help='(km) above sea level if itype=1, (km) from center of Earth if itype=2',type=float,nargs='+',default=[0])
    p.add_argument('-c','--latlon',help='geodetic latitude, longitude (deg)',type=float,nargs=2,default=(None,None))
    p = p.parse_args()

    # do world-wide grid if no user input
    if p.altkm is None or p.latlon[0] is None:
        glat,glon = latlonworldgrid()
    elif p.altkm is not None and p.latlon[0] is not None:
        glat,glon = p.latlon
    else:
        exit('please input all 3 of lat,lon,alt or none of them')

    x,y,z,f, yeardec = testigrf12(p.simtime,p.isv, p.itype, p.altkm, glat,glon)
    x11,y11,z11,f11 = testigrf11(p.simtime,p.isv,p.itype, p.altkm, glat,glon)

    if glat.ndim==2:
        plotigrf(x,y,z,f,glat,glon,yeardec,p.isv,'12')
        #plotigrf(x,y,z,f,glat,glon,p.year,p.isv,'11')

        #plotdiff1112(x,x11,y,y11,z,z11,f,f11,glat,glon,p.year,p.isv)
    else:
        print('x y z f')
        print(x,y,z,f)

    show()
