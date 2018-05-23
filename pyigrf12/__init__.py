"""
use IGRF12 and IGRF11 via f2py from Python
Michael Hirsch, Ph.D.
"""
import xarray
import numpy as np
import datetime
#
import sciencedates
#
import igrf12


def gridigrf12(date:datetime, glat:float, glon:float, alt:float, isv:int=0, itype:int=1) -> xarray.Dataset:

    glat = np.atleast_1d(glat)
    glon = np.atleast_1d(glon)

    yeardec = sciencedates.datetime2yeardec(date)
    colat,elon = latlon2colat(glat.ravel(), glon.ravel())

    x = np.empty(colat.size)
    y = np.empty_like(x);
    z = np.empty_like(x)
    f = np.empty_like(x)

    for i,(clt,eln) in enumerate(zip(colat,elon)):
        x[i],y[i],z[i],f[i] = igrf12.igrf12syn(isv, yeardec, itype, alt, clt, eln)
# %% assemble output
    if glat.ndim==2 and glon.ndim==2: # assume meshgrid
        coords={'glat':glat[:,0], 'glon':glon[0,:]}
    elif glat.ndim==1 and glon.ndim==1:
        coords={'glat':glat, 'glon':glon}
    else:
        raise ValueError('unexpected glat/glon shapes {} {}'.format(glat.shape, glon.shape))


    mag = xarray.Dataset(coords = coords,
                         attrs={'time':date, 'isv':isv})
    mag['north'] = (('glat','glon'), x.reshape(glat.shape))
    mag['east'] = (('glat','glon'), y.reshape(glat.shape))
    mag['down'] = (('glat','glon'), z.reshape(glat.shape))
    mag['total'] = (('glat','glon'), f.reshape(glat.shape))

    decl, incl = mag_vector2incl_decl(mag.north, mag.east, mag.down)

    mag['incl'] = (('glat','glon'),incl)
    mag['decl'] = (('glat','glon'),decl)

    return mag


def igrf(date:datetime, glat:float, glon:float, alt:float, isv:int=0, itype:int=1, model:str='12') -> xarray.Dataset:
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
    elif isinstance(yeardec,float): # assume decimal year
        pass
    else:
        raise TypeError('unknown yeardec type {}'.format(type(yeardec)))

    colat, elon = latlon2colat(glat,glon)

    assert colat.size==elon.size==1

    alt = np.atleast_1d(alt)
    Bnorth = np.empty(alt.size)
    Beast = np.empty_like(Bnorth)
    Bvert = np.empty_like(Bnorth)
    Btotal = np.empty_like(Bnorth)
    for i,a in enumerate(alt):
        if model=='12':
            Bnorth[i], Beast[i], Bvert[i], Btotal[i] = igrf12.igrf12syn(isv, yeardec, itype, a, colat, elon)
        elif model=='11':
            import igrf11
            Bnorth[i], Beast[i], Bvert[i], Btotal[i] = igrf11.igrf11syn(isv, yeardec, itype, a, colat, elon)
        else:
            raise ValueError('unknown IGRF model {}'.format(model))
# %% assemble output
    mag = xarray.Dataset({'north':('alt_km',Bnorth),

                          'east':('alt_km',Beast),
                          'down':('alt_km',Bvert),
                          'total':('alt_km',Btotal)},
                          coords={'alt_km':alt})

    decl, incl = mag_vector2incl_decl(mag.north, mag.east, mag.down)

    mag['incl'] = ('alt_km',incl)
    mag['decl'] = ('alt_km',decl)

    return mag

# %% utility functions
def mag_vector2incl_decl(x:float,y:float,z:float) -> tuple:
    """
    Inputs:
    -------
    vectors of the geomagnetic field.
    x: north componment
    y: east component
    z: down (by convention) component

    outputs:
    --------
    declination [degrees]
    inclination [degrees]

    http://geomag.nrcan.gc.ca/mag_fld/comp-en.php
    """

    decl = np.degrees(np.arctan2(y,x))

    incl = np.degrees(np.arctan2(z, np.hypot(x,y)))

    return decl, incl

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
