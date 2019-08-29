from typing import Tuple
import numpy as np


# %% utility functions
def mag_vector2incl_decl(x: float, y: float, z: float) -> Tuple[float, float]:
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


def latlon2colat(glat: float, glon: float) -> Tuple[np.ndarray, np.ndarray]:
    # atleast_1d for iteration later
    colat = 90 - np.atleast_1d(glat)
    elon = (360 + np.atleast_1d(glon)) % 360

    return colat, elon


def latlonworldgrid(latstep: int = 5, lonstep: int = 5) -> Tuple[np.ndarray, np.ndarray]:
    lat = np.arange(-90.0, 90 + latstep, latstep)
    lon = np.arange(-180.0, 180 + lonstep, lonstep)
    glon, glat = np.meshgrid(lon, lat)

    return glat, glon
