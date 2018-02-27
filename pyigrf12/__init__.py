"""
Quick demo of calling IGRF12 and IGRF11 using f2py from Python
Michael Hirsch, Ph.D.
"""
import numpy as np
import datetime
#
import sciencedates
#
import igrf12


def gridigrf12(date:datetime, glat:float, glon:float, alt:float, isv:int=0, itype:int=1):

    yeardec = sciencedates.datetime2yeardec(date)
    colat,elon = latlon2colat(glat.ravel(), glon.ravel())

    x = np.empty(colat.size)
    y = np.empty_like(x);
    z = np.empty_like(x)
    f = np.empty_like(x)

    for i,(clt,eln) in enumerate(zip(colat,elon)):
        x[i],y[i],z[i],f[i] = igrf12.igrf12syn(isv, yeardec, itype, alt, clt, eln)

    return x.reshape(glat.shape), y.reshape(glat.shape), z.reshape(glat.shape),f.reshape(glat.shape), yeardec


def runigrf12(date:datetime, glat:float, glon:float, alt:float, isv:int=0, itype:int=1):
    """
    date: datetime.date or decimal year yyyy.dddd
    glat, glon: geographic Latitude, Longitude
    alt: altitude [km] above sea level for itype==1
    isv: 0 for main geomagnetic field
    itype: 1: altitude is above sea level
    """

    # decimal year
    if isinstance(date,(datetime.date, datetime.datetime)):
        yeardec = sciencedates.datetime2yeardec(date)
    elif isinstance(yeardec,float): # assume decimate year
        pass
    else:
        raise TypeError(f'unknown yeardec type {type(yeardec)}')

    colat, elon = latlon2colat(glat,glon)

    assert colat.size==elon.size==1

    alt = np.atleast_1d(alt)
    Bnorth = np.empty(alt.size)
    Beast = np.empty_like(Bnorth)
    Bdown = np.empty_like(Bnorth)
    Btotal = np.empty_like(Bnorth)
    for i,a in enumerate(alt):
        Bnorth[i], Beast[i], Bdown[i], Btotal[i] = igrf12.igrf12syn(isv, yeardec, itype, a, colat, elon)

    return Bnorth, Beast, Bdown, Btotal


def runigrf11(date:datetime, glat:float, glon:float, alt:float, isv=0, itype=1):
    import igrf11

    yeardec = sciencedates.datetime2yeardec(date)
    colat,elon = latlon2colat(glat, glon)

    alt = np.atleast_1d(alt)
    Bnorth = np.empty(alt.size)
    Beast = np.empty_like(Bnorth)
    Bdown = np.empty_like(Bnorth)
    Btotal = np.empty_like(Bnorth)
    for i,a in enumerate(alt):
        Bnorth[i], Beast[i], Bdown[i], Btotal[i] = igrf11.igrf11syn(isv, yeardec, itype, a, colat, elon)

    return Bnorth, Beast, Bdown, Btotal


def latlon2colat(glat:float, glon:float):
    #atleast_1d for iteration later
    colat = 90 - np.atleast_1d(glat)
    elon = (360 + np.atleast_1d(glon)) % 360

    return colat,elon


def latlonworldgrid(latstep:int=5, lonstep:int=5):
    lat = np.arange(-90.,90+latstep,latstep)
    lon = np.arange(-180.,180+lonstep,lonstep)
    glon,glat = np.meshgrid(lon,lat)
    return glat,glon
