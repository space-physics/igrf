#!/usr/bin/env python
import datetime
import numpy as np
#
import pyigrf12

def test_igrf():
    dt = datetime.datetime(2012,7,12,12)

    mag = pyigrf12.runigrf(dt,65,85,0)

    np.testing.assert_allclose(mag.Bnorth,9218.3206585840526)
    np.testing.assert_allclose(mag.Beast, 2530.4855755646345)
    np.testing.assert_allclose(mag.Bvert,59711.688907276119)
    np.testing.assert_allclose(mag.Btotal,60472.031429181705)

if __name__ == '__main__':
    np.testing.run_module_suite()
