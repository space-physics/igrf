#!/usr/bin/env python
import datetime
import numpy as np
#
import pyigrf12

def test_igrf12():
    dt = datetime.datetime(2012,7,12,12)

    mag = pyigrf12.igrf(dt,65,85,0)

    np.testing.assert_allclose(mag.north,9218.3206585840526)
    np.testing.assert_allclose(mag.east, 2530.4855755646345)
    np.testing.assert_allclose(mag.down,59711.688907276119)
    np.testing.assert_allclose(mag.total,60472.031429181705)

    np.testing.assert_allclose(mag.incl, 80.904615)
    np.testing.assert_allclose(mag.decl, 15.349941)

if __name__ == '__main__':
    np.testing.run_module_suite()
