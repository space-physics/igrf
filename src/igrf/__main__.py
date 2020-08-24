#!/usr/bin/env python
import datetime
from argparse import ArgumentParser
import numpy as np

import igrf


try:
    import igrf.plots as plt
    from matplotlib.pyplot import show
except ImportError:
    show = plt = None


def cli():

    p = ArgumentParser(
        description="calls IGRF from Python, and plots " "the modeled geomagnetic field"
    )
    p.add_argument("date", help="date of sim", nargs="?", default=datetime.date.today())
    p.add_argument("--isv", help="0: main field. 1: secular variation", type=int, default=0)
    p.add_argument("--itype", help="1: geodetic. 2: geocentric", type=int, default=1)
    p.add_argument(
        "-a",
        "--altkm",
        help="(km) above sea level if itype=1," " (km) from center of Earth if itype=2",
        type=float,
        nargs="+",
        default=0,
    )
    p.add_argument("-c", "--latlon", help="geodetic latitude, longitude (deg)", type=float, nargs=2)
    P = p.parse_args()

    # do world-wide grid if no user input
    if not P.altkm or not P.latlon:
        glat, glon = igrf.utils.latlonworldgrid()
        mag = igrf.grid(P.date, glat, glon, P.altkm, isv=P.isv, itype=P.itype)
    elif P.altkm and P.latlon:
        glat, glon = P.latlon
        mag = igrf.igrf(P.date, glat, glon, P.altkm, isv=P.isv, itype=P.itype)
    else:
        raise ValueError("please input all 3 of lat,lon,alt or none of them")

    if show is not None and isinstance(glat, np.ndarray) and glat.ndim == 2:
        plt.plotigrf(mag, "13")
        show()
    else:
        print(mag)
