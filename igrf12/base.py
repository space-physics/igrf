from typing import Union
import xarray
import sciencedates
from datetime import datetime, date
import igrf12fort
import numpy as np
from .utils import latlon2colat, mag_vector2incl_decl
try:
    import igrf11fort
except ImportError:
    igrf11fort = None


def gridigrf12(t: datetime,
               glat: Union[np.ndarray, float], glon: Union[np.ndarray, float],
               alt_km: Union[np.ndarray, float],
               isv: int = 0, itype: int = 1) -> xarray.Dataset:

    glat = np.atleast_1d(glat)
    glon = np.atleast_1d(glon)

    yeardec = sciencedates.datetime2yeardec(t)
    colat, elon = latlon2colat(glat.ravel(), glon.ravel())

    x = np.empty(colat.size)
    y = np.empty_like(x)
    z = np.empty_like(x)
    f = np.empty_like(x)

    for i, (clt, eln) in enumerate(zip(colat, elon)):
        x[i], y[i], z[i], f[i] = igrf12fort.igrf12syn(isv, yeardec, itype, alt_km, clt, eln)
# %% assemble output
    if glat.ndim == 2 and glon.ndim == 2:  # assume meshgrid
        coords = {'glat': glat[:, 0], 'glon': glon[0, :]}
    elif glat.ndim == 1 and glon.ndim == 1:
        coords = {'glat': glat, 'glon': glon}
    else:
        raise ValueError(f'glat/glon shapes: {glat.shape} {glon.shape}')

    mag = xarray.Dataset(coords=coords,
                         attrs={'time': t, 'isv': isv})
    mag['north'] = (('glat', 'glon'), x.reshape(glat.shape))
    mag['east'] = (('glat', 'glon'), y.reshape(glat.shape))
    mag['down'] = (('glat', 'glon'), z.reshape(glat.shape))
    mag['total'] = (('glat', 'glon'), f.reshape(glat.shape))

    decl, incl = mag_vector2incl_decl(mag.north, mag.east, mag.down)

    mag['incl'] = (('glat', 'glon'), incl)
    mag['decl'] = (('glat', 'glon'), decl)

    return mag


def igrf(time: datetime, glat: Union[np.ndarray, float],
         glon: Union[np.ndarray, float],
         alt_km: Union[np.ndarray, float],
         isv: int = 0, itype: int = 1, model: int = 12) -> xarray.Dataset:
    """
    date: datetime.date or decimal year yyyy.dddd
    glat, glon: geographic Latitude, Longitude
    alt_km: altitude [km] above sea level for itype==1
    isv: 0 for main geomagnetic field
    itype: 1: altitude is above sea level
    """

    # decimal year
    if isinstance(time, (str, date, datetime)):
        yeardec = sciencedates.datetime2yeardec(time)
    elif isinstance(time, float):  # assume decimal year
        yeardec = time
    else:
        raise TypeError(f'unknown time format {type(time)}')

    colat, elon = latlon2colat(glat, glon)

    assert colat.size == elon.size == 1

    alt_km = np.atleast_1d(alt_km)
    Bnorth = np.empty(alt_km.size)
    Beast = np.empty_like(Bnorth)
    Bvert = np.empty_like(Bnorth)
    Btotal = np.empty_like(Bnorth)
    for i, a in enumerate(alt_km):
        if model == 12:
            Bnorth[i], Beast[i], Bvert[i], Btotal[i] = igrf12fort.igrf12syn(isv,
                                                                            yeardec, itype, a,  colat, elon)
        elif model == 11:
            Bnorth[i], Beast[i], Bvert[i], Btotal[i] = igrf11fort.igrf11syn(isv,
                                                                            yeardec, itype, a, colat, elon)
        else:
            raise ValueError(f'unknown IGRF model {model}')
# %% assemble output
    mag = xarray.Dataset({'north': ('alt_km', Bnorth),
                          'east': ('alt_km', Beast),
                          'down': ('alt_km', Bvert),
                          'total': ('alt_km', Btotal)},
                         coords={'alt_km': alt_km})

    decl, incl = mag_vector2incl_decl(mag.north, mag.east, mag.down)

    mag['incl'] = ('alt_km', incl)
    mag['decl'] = ('alt_km', decl)

    return mag
