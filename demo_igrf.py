from matplotlib.pyplot import show
#
from gridaurora.worldgrid import latlonworldgrid
from igrf12py.igrf12fun import runigrf12,plotigrf

if __name__ == '__main__':
    from argparse import ArgumentParser
    p = ArgumentParser(description='calls IGRF from Python, a basic demo')
    p.add_argument('simtime',help='yyyy-mm-ddTHH:MM:SSZ time of sim',nargs='?',default='')
    p.add_argument('--isv',help='0: main field. 1: secular variation',type=int,default=0)
    p.add_argument('--itype',help='1: geodetic. 2: geocentric',type=int,default=1)
    p.add_argument('-a','--altkm',help='(km) above sea level if itype=1, (km) from center of Earth if itype=2',type=float,nargs='+',default=[0])
    p.add_argument('-c','--latlon',help='geodetic latitude, longitude (deg)',type=float,nargs=2)
    p = p.parse_args()


    # do world-wide grid if no user input
    if not p.altkm or not p.latlon:
        glat,glon = latlonworldgrid()
    elif p.altkm and p.latlon:
        glat,glon = p.latlon
    else:
        exit('please input all 3 of lat,lon,alt or none of them')

    x,y,z,f, yeardec = runigrf12(p.simtime,p.isv, p.itype, p.altkm, glat,glon)
#    x11,y11,z11,f11 = testigrf11(p.simtime,p.isv,p.itype, p.altkm, glat,glon)

    if glat.ndim==2:
        plotigrf(x,y,z,f,glat,glon,yeardec,p.isv,'12')
        #plotigrf(x,y,z,f,glat,glon,p.year,p.isv,'11')

        #plotdiff1112(x,x11,y,y11,z,z11,f,f11,glat,glon,p.year,p.isv)
    else:
        print('x y z f')
        print(x,y,z,f)

    show()