#!/usr/bin/env python
from datetime import datetime
from numpy.testing import assert_allclose,run_module_suite
#
from pyigrf12 import runigrf12

def test_igrf():
    dt = datetime(2012,7,12,12)

    x,y,z,f = runigrf12(dt,0,1,0,65,85)

    assert_allclose((x.squeeze(),y.squeeze(),z.squeeze(),f.squeeze()),
                (9218.3206585840526,2530.4855755646345,59711.688907276119,60472.031429181705))

if __name__ == '__main__':
    run_module_suite()
