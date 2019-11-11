"""
use IGRF12 and IGRF11 via f2py from Python
Michael Hirsch, Ph.D.
"""
from .base import igrf, gridigrf12
from .utils import mag_vector2incl_decl, latlon2colat, latlonworldgrid
