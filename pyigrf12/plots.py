from matplotlib.pyplot import figure,subplots
from matplotlib.ticker import ScalarFormatter
#
sfmt = ScalarFormatter(useMathText=True) #for 10^3 instead of 1e3
sfmt.set_powerlimits((-2, 2))
sfmt.set_scientific(True)
sfmt.set_useOffset(False)

def plotigrf(x,y,z,f,glat,glon,year,isv,mdl):
    fg,ax = subplots(2,2,sharex=True)
    fg.suptitle(str(year))
    ax = ax.ravel()
    for a,i,j in zip(ax,(x,y,z),('x','y','z')):
        hi = a.pcolormesh(glon,glat,i,
                      cmap='bwr',
                      vmin=-6e4,vmax=6e4) #symmetrix vmin,vmax centers white at zero for bwr cmap
        fg.colorbar(hi,ax=a,format=sfmt)
        a.set_title(f'IGRF{mdl} $B_{j}$-field')

    for a in ax[[0,2]]:
        a.set_ylabel('latitude (deg)')
    for a in ax[[2,3]]:
        a.set_xlabel('longitude (deg)')


    if isv==0:
        hi = a.pcolormesh(glon,glat,f)
        fg.colorbar(hi,ax=a,format=sfmt)
        a.set_title('IGRF{} $B$-field: total intensity [nT] on {:.2f}'.format(mdl,year))

def plotdiff1112(x,x11,y,y11,z,z11,f,f11,glat,glon,year,isv):
    for i,j,k in zip((x,y,z),(x11,y11,z11),('x','y','z')):
        fg = figure()
        ax = fg.gca()
        hi = ax.imshow(i-j,extent=(glon[0,0],glon[0,-1],glat[0,0],glat[-1,0]))
        fg.colorbar(hi,format=sfmt)
        ax.set_ylabel('latitude (deg)')
        ax.set_xlabel('longitude (deg)')
        ax.set_title('IGRF12-IGRF11 $B_{}$-field comparison on {:.2f}'.format(k,year))

    if isv==0:
        fg = figure()
        ax = fg.gca()
        hi = ax.imshow(f-f11,extent=(glon[0,0],glon[0,-1],glat[0,0],glat[-1,0]))
        fg.colorbar(hi)
        ax.set_xlabel('latitude (deg)')
        ax.set_ylabel('longitude (deg)')
        ax.set_title('IGRF12-IGRF11 $B$-field: comparison total intensity [nT] on {:.2f}'.format(year))
