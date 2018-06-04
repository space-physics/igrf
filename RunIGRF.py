#!/usr/bin/env python
from matplotlib.pyplot import show
import datetime
import xarray
#
import pyigrf12
import pyigrf12.plots as plt


if __name__ == '__main__':
    from argparse import ArgumentParser
    p = ArgumentParser(description='calls IGRF from Python, and plots '
                       'the modeled geomagnetic field')
    p.add_argument('date', help='date of sim', nargs='?',
                   default=datetime.date.today())
    p.add_argument('--isv', help='0: main field. 1: secular variation',
                   type=int, default=0)
    p.add_argument('--itype', help='1: geodetic. 2: geocentric', type=int,
                   default=1)
    p.add_argument('-a', '--altkm',
                   help='(km) above sea level if itype=1,'
                        ' (km) from center of Earth if itype=2', type=float,
                   nargs='+', default=0)
    p.add_argument('-c', '--latlon', help='geodetic latitude, longitude (deg)',
                   type=float, nargs=2)
    P = p.parse_args()

    # do world-wide grid if no user input
    if not P.altkm or not P.latlon:
        glat, glon = pyigrf12.latlonworldgrid()
    elif P.altkm and P.latlon:
        glat, glon = P.latlon
    else:
        raise ValueError('please input all 3 of lat,lon,alt or none of them')

    mag12: xarray.Dataset = pyigrf12.gridigrf12(P.date, glat, glon, P.altkm,
                                                P.isv, P.itype)
#   mag11 = pyigrf12.testigrf11(p.date,glat,glon,p.altkm, p.isv, p.itype)

    if glat.ndim == 2:
        plt.plotigrf(mag12, '12')

        # plt.plotdiff1112(mag12,mag11)
    else:
        print(mag12)

    show()
