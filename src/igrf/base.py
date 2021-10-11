import xarray
from datetime import datetime
import numpy as np
import subprocess
import shutil
import os
import importlib.resources

from .utils import mag_vector2incl_decl, datetime2yeardec

NAME = "igrf13_driver"
if os.name == "nt":
    NAME += ".exe"


def build():
    """
    attempt to build using CMake
    """
    cmake = shutil.which("cmake")
    if not cmake:
        raise FileNotFoundError("CMake not available")

    with importlib.resources.path(__package__, "CMakeLists.txt") as f:
        s = f.parent
        b = s / "build"
        g = []

        if os.name == "nt" and not os.environ.get("CMAKE_GENERATOR"):
            g = ["-G", "MinGW Makefiles"]

        subprocess.check_call([cmake, f"-S{s}", f"-B{b}"] + g)
        subprocess.check_call([cmake, "--build", str(b), "--parallel"])


def grid(
    time: datetime,
    glat: np.ndarray,
    glon: np.ndarray,
    alt_km: np.ndarray,
    *,
    isv: int = 0,
    itype: int = 1,
) -> xarray.Dataset:

    glat = np.atleast_1d(glat)
    glon = np.atleast_1d(glon)

    yeardec = datetime2yeardec(time)

    x = np.empty(glat.size)
    y = np.empty_like(x)
    z = np.empty_like(x)
    f = np.empty_like(x)

    try:
        with importlib.resources.path(__package__, NAME) as exe:
            pass
    except FileNotFoundError:
        build()

    with importlib.resources.path(__package__, NAME) as exe:
        for i, (la, lo) in enumerate(zip(glat.ravel(), glon.ravel())):
            cmd = [str(exe), str(yeardec), str(la), str(lo), str(alt_km), str(isv), str(itype)]
            ret = subprocess.run(cmd, text=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            if ret.returncode != 0:
                raise RuntimeError(
                    f"IGRF13 error code {ret.returncode}\n{ret.stderr}\n{' '.join(cmd)}"
                )
            # different compilers throw in extra \n
            x[i], y[i], z[i], f[i] = list(map(float, ret.stdout.split()))

    # %% assemble output
    if glat.ndim == 2 and glon.ndim == 2:  # assume meshgrid
        coords = {"glat": glat[:, 0], "glon": glon[0, :]}
    elif glat.ndim == 1 and glon.ndim == 1:
        coords = {"glat": glat, "glon": glon}
    else:
        raise ValueError(f"glat/glon shapes: {glat.shape} {glon.shape}")

    mag = xarray.Dataset(coords=coords, attrs={"time": time, "isv": isv, "itype": itype})
    mag["north"] = (("glat", "glon"), x.reshape(glat.shape))
    mag["east"] = (("glat", "glon"), y.reshape(glat.shape))
    mag["down"] = (("glat", "glon"), z.reshape(glat.shape))
    mag["total"] = (("glat", "glon"), f.reshape(glat.shape))

    decl, incl = mag_vector2incl_decl(mag.north, mag.east, mag.down)

    mag["incl"] = (("glat", "glon"), incl)
    mag["decl"] = (("glat", "glon"), decl)

    return mag


def igrf(
    time: datetime,
    glat: float,
    glon: float,
    alt_km: np.ndarray,
    *,
    isv: int = 0,
    itype: int = 1,
) -> xarray.Dataset:
    """

    Parameters
    ----------

    date: datetime.date or decimal year yyyy.dddd
    glat, glon: geographic Latitude, Longitude
    alt_km: altitude [km] above sea level for itype==1
    isv: 0 for main geomagnetic field
    itype: 1: altitude is above sea level
    """

    # decimal year
    yeardec = datetime2yeardec(time)

    alt_km = np.atleast_1d(alt_km)
    Bnorth = np.empty(alt_km.size)
    Beast = np.empty_like(Bnorth)
    Bvert = np.empty_like(Bnorth)
    Btotal = np.empty_like(Bnorth)

    try:
        with importlib.resources.path(__package__, NAME) as exe:
            pass
    except FileNotFoundError:
        build()

    with importlib.resources.path(__package__, NAME) as exe:
        for i, a in enumerate(alt_km):
            cmd = [str(exe), str(yeardec), str(glat), str(glon), str(a), str(isv), str(itype)]
            ret = subprocess.run(cmd, text=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            if ret.returncode != 0:
                raise RuntimeError(
                    f"IGRF13 error code {ret.returncode}\n{ret.stderr}\n{' '.join(cmd)}"
                )
            # different compilers throw in extra \n

            Bnorth[i], Beast[i], Bvert[i], Btotal[i] = list(map(float, ret.stdout.split()))

    # %% assemble output
    mag = xarray.Dataset(
        {
            "north": ("alt_km", Bnorth),
            "east": ("alt_km", Beast),
            "down": ("alt_km", Bvert),
            "total": ("alt_km", Btotal),
        },
        coords={"alt_km": alt_km},
        attrs={"time": time, "isv": isv, "itype": itype, "glat": glat, "glon": glon},
    )

    decl, incl = mag_vector2incl_decl(mag.north.data, mag.east.data, mag.down.data)

    mag["incl"] = ("alt_km", incl)
    mag["decl"] = ("alt_km", decl)

    return mag
