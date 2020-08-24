import typing
import datetime
from dateutil.parser import parse
import numpy as np


# %% utility functions
def mag_vector2incl_decl(x: float, y: float, z: float) -> typing.Tuple[float, float]:
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

    decl = np.degrees(np.arctan2(y, x))

    incl = np.degrees(np.arctan2(z, np.hypot(x, y)))

    return decl, incl


def latlon2colat(glat: float, glon: float) -> typing.Tuple[np.ndarray, np.ndarray]:
    # atleast_1d for iteration later
    colat = 90 - np.atleast_1d(glat)
    elon = (360 + np.atleast_1d(glon)) % 360

    return colat, elon


def latlonworldgrid(latstep: int = 5, lonstep: int = 5) -> typing.Tuple[np.ndarray, np.ndarray]:
    lat = np.arange(-90.0, 90 + latstep, latstep)
    lon = np.arange(-180.0, 180 + lonstep, lonstep)
    glon, glat = np.meshgrid(lon, lat)

    return glat, glon


def datetime2yeardec(time: typing.Union[str, datetime.datetime, datetime.date]) -> float:
    """
    Convert a datetime into a float. The integer part of the float should
    represent the year.
    Order should be preserved. If adate<bdate, then d2t(adate)<d2t(bdate)
    time distances should be preserved: If bdate-adate=ddate-cdate then
    dt2t(bdate)-dt2t(adate) = dt2t(ddate)-dt2t(cdate)
    """

    if isinstance(time, float):
        # assume already year_dec
        return time
    if isinstance(time, str):
        t = parse(time)
    elif isinstance(time, datetime.datetime):
        t = time
    elif isinstance(time, datetime.date):
        t = datetime.datetime.combine(time, datetime.datetime.min.time())
    elif isinstance(time, (tuple, list, np.ndarray)):
        return np.asarray([datetime2yeardec(t) for t in time])
    else:
        raise TypeError("unknown input type {}".format(type(time)))

    year = t.year

    boy = datetime.datetime(year, 1, 1)
    eoy = datetime.datetime(year + 1, 1, 1)

    return year + ((t - boy).total_seconds() / ((eoy - boy).total_seconds()))
