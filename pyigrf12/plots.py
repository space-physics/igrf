from matplotlib.pyplot import figure
from matplotlib.ticker import ScalarFormatter
#
sfmt = ScalarFormatter(useMathText=True) #for 10^3 instead of 1e3
sfmt.set_powerlimits((-2, 2))
sfmt.set_scientific(True)
sfmt.set_useOffset(False)

def plotigrf(mag, model):
    fg = figure(figsize=(10,8))
    ax = fg.subplots(2,2,sharex=True)

    fg.suptitle('IGRF{} {}'.format(model,mag.time))
    ax = ax.ravel()
    for a,i in zip(ax,('Bnorth','Beast','Bvert')):
        hi = a.pcolormesh(mag.glon,mag.glat,mag[i],
                      cmap='bwr',
                      vmin=-6e4,vmax=6e4) #symmetrix vmin,vmax centers white at zero for bwr cmap
        fg.colorbar(hi,ax=a,format=sfmt)
        a.set_title('{} [nT]'.format(i))

    for a in ax[[0,2]]:
        a.set_ylabel('latitude (deg)')
    for a in ax[[2,3]]:
        a.set_xlabel('longitude (deg)')


    if mag.isv==0:
        hi = a.pcolormesh(mag.glon, mag.glat, mag['Btotal'])
        fg.colorbar(hi,ax=a,format=sfmt)
        a.set_title('$B$ total intensity [nT]')

def plotdiff1112(mag12,mag11):
    for i in ('x','y','z'):
        fg = figure()
        ax = fg.gca()
        hi = ax.imshow(mag12[i]-mag11[i],
                    extent=(mag.glon[0],mag.glon[-1],mag.glat[0],mag.glat[-1]))
        fg.colorbar(hi,format=sfmt)
        ax.set_ylabel('latitude (deg)')
        ax.set_xlabel('longitude (deg)')
        ax.set_title('IGRF12-IGRF11 {}-field comparison on {:.2f}'.format(i,year))

    if isv==0:
        fg = figure()
        ax = fg.gca()
        hi = ax.imshow(mag12['Btotal'] - mag11['Btotal'],
                extent=(mag.glon[0],mag.glon[-1],mag.glat[0],mag.glat[-1]))
        fg.colorbar(hi)
        ax.set_xlabel('latitude (deg)')
        ax.set_ylabel('longitude (deg)')
        ax.set_title('IGRF12-IGRF11 $B$-field: comparison total intensity [nT] on {:.2f}'.format(year))
