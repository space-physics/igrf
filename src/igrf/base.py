import xarray
from datetime import datetime, date
import numpy as np
import subprocess
import shutil
import typing as T
from pathlib import Path
import f90nml

from .utils import latlon2colat, mag_vector2incl_decl, datetime2yeardec

Rs = Path(__file__).resolve().parents[2]
exe = shutil.which("igrf_run", path=str(Rs))
if not exe:
    subprocess.check_call(["ctest", "-S", str(Rs / "setup.cmake"), "-VV"])
exe = shutil.which("igrf_run", path=str(Rs / "build"))
if not exe:
    raise ImportError("could not find IGRF driver, was it built?")


def grid(
    t: datetime, glat: np.ndarray, glon: np.ndarray, alt_km: np.ndarray, model: int = 13
) -> xarray.Dataset:

    glat = np.atleast_1d(glat)
    glon = np.atleast_1d(glon)

    yeardec = datetime2yeardec(t)
    colat, elon = latlon2colat(glat.ravel(), glon.ravel())

    x = np.empty(colat.size)
    y = np.empty_like(x)
    z = np.empty_like(x)
    f = np.empty_like(x)

    x, y, z, f = igrf_run(model, yeardec, alt_km, colat, elon)
    # %% assemble output
    if glat.ndim == 2 and glon.ndim == 2:  # assume meshgrid
        coords = {"glat": glat[:, 0], "glon": glon[0, :]}
    elif glat.ndim == 1 and glon.ndim == 1:
        coords = {"glat": glat, "glon": glon}
    else:
        raise ValueError(f"glat/glon shapes: {glat.shape} {glon.shape}")

    mag = xarray.Dataset(coords=coords, attrs={"time": t, "isv": isv})
    mag["north"] = (("glat", "glon"), x.reshape(glat.shape))
    mag["east"] = (("glat", "glon"), y.reshape(glat.shape))
    mag["down"] = (("glat", "glon"), z.reshape(glat.shape))
    mag["total"] = (("glat", "glon"), f.reshape(glat.shape))

    decl, incl = mag_vector2incl_decl(mag.north, mag.east, mag.down)

    mag["incl"] = (("glat", "glon"), incl)
    mag["decl"] = (("glat", "glon"), decl)

    return mag


def igrf_run(
    model: int, yeardec: float, alt_km: float, colat: float, eastlon: float
) -> T.Dict[str, T.Any]:

    nml = f90nml.namelist.Namelist({"date": yeardec, "alt": alt_km, "clt": colat, "xln": eastlon})
    nml.write("in.nml", force=True)

    subprocess.check_call([str(exe), "in.nml", "out.nml"])

    return f90nml.read("out.nml").todict()


def igrf(
    time: datetime, glat: np.ndarray, glon: np.ndarray, alt_km: np.ndarray, model: int = 13,
) -> xarray.Dataset:
    """
    date: datetime.date or decimal year yyyy.dddd
    glat, glon: geographic Latitude, Longitude
    alt_km: altitude [km] above sea level for itype==1
    isv: 0 for main geomagnetic field
    itype: 1: altitude is above sea level
    """

    # decimal year
    if isinstance(time, (str, date, datetime)):
        yeardec = datetime2yeardec(time)
    elif isinstance(time, float):  # assume decimal year
        yeardec = time
    else:
        raise TypeError(f"unknown time format {type(time)}")

    colat, elon = latlon2colat(glat, glon)

    assert colat.size == elon.size == 1

    alt_km = np.atleast_1d(alt_km)
    Bnorth = np.empty(alt_km.size)
    Beast = np.empty_like(Bnorth)
    Bvert = np.empty_like(Bnorth)
    Btotal = np.empty_like(Bnorth)
    for i, a in enumerate(alt_km):
        Bnorth[i], Beast[i], Bvert[i], Btotal[i] = igrf_run(model, yeardec, a, colat, elon)

    # %% assemble output
    mag = xarray.Dataset(
        {
            "north": ("alt_km", Bnorth),
            "east": ("alt_km", Beast),
            "down": ("alt_km", Bvert),
            "total": ("alt_km", Btotal),
        },
        coords={"alt_km": alt_km},
    )

    decl, incl = mag_vector2incl_decl(mag.north, mag.east, mag.down)

    mag["incl"] = ("alt_km", incl)
    mag["decl"] = ("alt_km", decl)

    return mag
