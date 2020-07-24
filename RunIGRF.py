#!/usr/bin/env python
from matplotlib.pyplot import show
import xarray
import argparse

import igrf
import igrf.plots as plt


def main():

    p = argparse.ArgumentParser(
        description="calls IGRF from Python, and plots " "the modeled geomagnetic field"
    )
    p.add_argument("date", help="date of sim  e.g. 2012-05-12")
    p.add_argument(
        "-a", "--altkm", help="(km) above sea level", type=float, nargs="+", default=0,
    )
    p.add_argument("-c", "--latlon", help="geodetic latitude, longitude (deg)", type=float, nargs=2)
    p.add_argument("-m", "--model", help="IGRF model", type=int, choices=[13, 12, 11])
    P = p.parse_args()

    # do world-wide grid if no user input
    if not P.altkm or not P.latlon:
        glat, glon = igrf.latlonworldgrid()
    elif P.altkm and P.latlon:
        glat, glon = P.latlon
    else:
        raise ValueError("please input all 3 of lat,lon,alt or none of them")

    mag: xarray.Dataset = igrf.grid(P.date, glat, glon, P.altkm, model=13)

    if glat.ndim == 2:
        plt.plotigrf(mag, 13)
    else:
        print(mag)

    show()


if __name__ == "__main__":
    main()
