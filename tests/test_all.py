#!/usr/bin/env python
import datetime
import numpy as np
import unittest
#
import pyigrf12

class BasicTests(unittest.TestCase):

    def test_igrf(self):
        dt = datetime.datetime(2012,7,12,12)

        x,y,z,f = pyigrf12.runigrf12(dt,65,85,0)

        np.testing.assert_allclose((x.squeeze(),y.squeeze(),z.squeeze(),f.squeeze()),
                    (9218.3206585840526,2530.4855755646345,59711.688907276119,60472.031429181705))

if __name__ == '__main__':
    unittest.main()
