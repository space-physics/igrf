from matplotlib.pyplot import figure
from matplotlib.ticker import ScalarFormatter
import xarray

sfmt = ScalarFormatter(useMathText=True)  # for 10^3 instead of 1e3
sfmt.set_powerlimits((-2, 2))
sfmt.set_scientific(True)
sfmt.set_useOffset(False)


def plotigrf(mag: xarray.Dataset, model: str):
    mode = "contour"
    fg = figure(figsize=(10, 8))
    ax = fg.subplots(2, 2, sharex=True)

    fg.suptitle(f"IGRF{model} {mag.time}")
    ax = ax.ravel()
    for a, i in zip(ax, ("north", "east", "down")):
        if mode == "pcolor":
            # symmetric vmin,vmax centers white at zero: bwr cmap
            hi = a.pcolormesh(mag.glon, mag.glat, mag[i], cmap="bwr", vmin=-6e4, vmax=6e4)
            fg.colorbar(hi, ax=a, format=sfmt)
        elif mode == "contour":
            hi = a.contour(mag.glon, mag.glat, mag[i])
            a.clabel(hi, inline=True, fmt="%0.1f")
        else:
            raise ValueError(f"unknown plot type {mode}")

        a.set_title("{} [nT]".format(i))

    for a in ax[[0, 2]]:
        a.set_ylabel("Geographic latitude (deg)")
    for a in ax[[2, 3]]:
        a.set_xlabel("Geographic longitude (deg)")

    if mag.isv == 0:
        if mode == "pcolor":
            hi = a.pcolormesh(mag.glon, mag.glat, mag["total"])
            fg.colorbar(hi, ax=a, format=sfmt)
        elif mode == "contour":
            hi = a.contour(mag.glon, mag.glat, mag.total)
            a.clabel(hi, inline=True, fmt="%0.1f")
        else:
            raise ValueError(f"unknown plot type {mode}")

        a.set_title("$B$ total intensity [nT]")

    # %% incl, decl
    fg = figure()
    fg.suptitle(f"IGRF{model} {mag.time}")
    ax = fg.subplots(1, 2, sharey=True)

    hi = ax[0].contour(mag.glon, mag.glat, mag.decl, range(-90, 90 + 20, 20))
    ax[0].clabel(hi, inline=True, fmt="%0.1f")
    ax[0].set_title("Magnetic Declination [degrees]")

    hi = ax[1].contour(mag.glon, mag.glat, mag.incl, range(-90, 90 + 20, 20))
    ax[1].clabel(hi, inline=True, fmt="%0.1f")
    ax[1].set_title("Magnetic Inclination [degrees]")

    ax[0].set_ylabel("Geographic latitude (deg)")
    for a in ax:
        a.set_xlabel("Geographic longitude (deg)")


def plotdiff1112(mag12: xarray.Dataset, mag11: xarray.Dataset):
    for i in ("x", "y", "z"):
        fg = figure()
        ax = fg.gca()
        hi = ax.imshow(
            mag12[i] - mag11[i],
            extent=(mag12.glon[0], mag12.glon[-1], mag12.glat[0], mag12.glat[-1]),
        )
        fg.colorbar(hi, format=sfmt)
        ax.set_ylabel("latitude (deg)")
        ax.set_xlabel("longitude (deg)")
        ax.set_title(f"IGRF12-IGRF11 {i}-field comparison on {mag12.time}")

    if mag12.isv == 0:
        fg = figure()
        ax = fg.gca()
        hi = ax.imshow(
            mag12["Btotal"] - mag11["Btotal"],
            extent=(mag12.glon[0], mag12.glon[-1], mag12.glat[0], mag12.glat[-1]),
        )
        fg.colorbar(hi)
        ax.set_xlabel("latitude (deg)")
        ax.set_ylabel("longitude (deg)")
        ax.set_title(f"IGRF12-IGRF11 $B$: total intensity [nT] {mag12.time}")
