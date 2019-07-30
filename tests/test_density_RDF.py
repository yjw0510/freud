from __future__ import division

import numpy as np
import numpy.testing as npt
import freud
import unittest
import util


class TestRDF(unittest.TestCase):
    def test_generateR(self):
        r_max = 51.23
        dr = 0.1
        for r_min in [0, 0.05, 0.1, 1.0, 3.0]:
            nbins = int((r_max - r_min) / dr)

            # make sure the radius for each bin is generated correctly
            r_list = np.zeros(nbins, dtype=np.float32)
            for i in range(nbins):
                r1 = i * dr + r_min
                r2 = r1 + dr
                r_list[i] = 2.0/3.0 * (r2**3.0 - r1**3.0) / (r2**2.0 - r1**2.0)
            rdf = freud.density.RDF(r_max, dr, r_min=r_min)
            npt.assert_allclose(rdf.R, r_list, rtol=1e-4, atol=1e-4)

    def test_attribute_access(self):
        r_max = 10.0
        dr = 1.0
        num_points = 100
        box_size = r_max*3.1
        box, points = util.make_box_and_random_points(
            box_size, num_points, True)
        rdf = freud.density.RDF(r_max, dr)

        # Test protected attribute access
        with self.assertRaises(AttributeError):
            rdf.RDF
        with self.assertRaises(AttributeError):
            rdf.box
        with self.assertRaises(AttributeError):
            rdf.n_r

        rdf.accumulate(box, points)

        # Test if accessible now
        rdf.RDF
        rdf.box
        rdf.n_r

        # reset
        rdf.reset()

        # Test protected attribute access
        with self.assertRaises(AttributeError):
            rdf.RDF
        with self.assertRaises(AttributeError):
            rdf.box
        with self.assertRaises(AttributeError):
            rdf.n_r

        rdf.compute(box, points)

        # Test if accessible now
        rdf.RDF
        rdf.box
        rdf.n_r

    def test_invalid_rdf(self):
        # Make sure that invalid RDF objects raise errors
        with self.assertRaises(ValueError):
            freud.density.RDF(r_max=-1, dr=0.1)
        with self.assertRaises(ValueError):
            freud.density.RDF(r_max=1, dr=0)
        with self.assertRaises(ValueError):
            freud.density.RDF(r_max=1, dr=0.1, r_min=2)

    def test_random_point(self):
        r_max = 10.0
        dr = 1.0
        num_points = 10000
        tolerance = 0.1
        box_size = r_max*3.1

        def ig_sphere(x, y, j):
            return 4/3*np.pi*np.trapz(y[:j]*(x[:j]+dr/2)**2, x[:j])

        for i, r_min in enumerate([0, 0.05, 0.1, 1.0, 3.0]):
            nbins = int((r_max - r_min) / dr)
            box, points = util.make_box_and_random_points(box_size, num_points)
            points.flags['WRITEABLE'] = False
            box = freud.box.Box.cube(box_size)
            test_set = util.make_raw_query_nlist_test_set(
                box, points, points, "ball", r_max, 0, True)
            for ts in test_set:
                rdf = freud.density.RDF(r_max, dr, r_min=r_min)

                if i < 3:
                    rdf.accumulate(box, ts[0], nlist=ts[1])
                else:
                    rdf.compute(box, ts[0], nlist=ts[1])
                self.assertTrue(rdf.box == box)
                correct = np.ones(nbins, dtype=np.float32)
                correct[0] = 0.0
                npt.assert_allclose(rdf.RDF, correct, atol=tolerance)

                # Numerical integration to compute the running coordination
                # number will be highly inaccurate, so we can only test up to
                # a limited precision. Also, since dealing with nonzero r_min
                # values requires extrapolation, we only test when r_min=0.
                if r_min == 0:
                    correct_cumulative = np.array(
                        [ig_sphere(rdf.R, rdf.RDF, j)
                            for j in range(1, nbins+1)])
                    npt.assert_allclose(rdf.n_r, correct_cumulative,
                                        rtol=tolerance*5)

    def test_repr(self):
        rdf = freud.density.RDF(10, 0.1, r_min=0.5)
        self.assertEqual(str(rdf), str(eval(repr(rdf))))

    def test_repr_png(self):
        r_max = 10.0
        dr = 1.0
        num_points = 10
        box_size = r_max*3.1
        box, points = util.make_box_and_random_points(box_size, num_points)
        rdf = freud.density.RDF(r_max, dr)

        with self.assertRaises(AttributeError):
            rdf.plot()
        self.assertEqual(rdf._repr_png_(), None)

        rdf.accumulate(box, points)
        rdf._repr_png_()

    def test_points_ne_query_points(self):
        r_max = 100.0
        dr = 1
        box_size = r_max*5
        box = freud.box.Box.square(box_size)

        rdf = freud.density.RDF(r_max, dr)

        query_points = []
        supposed_RDF = [0]
        N = 100

        # With points closely centered around the origin,
        # the cumulative average bin counts should be same as
        # having a single point at the origin.
        # Also, we can check for whether points are not considered against
        # each other.
        points = [[dr/4, 0, 0], [-dr/4, 0, 0], [0, dr/4, 0], [0, -dr/4, 0]]
        for r in rdf.R:
            for k in range(N):
                query_points.append([r * np.cos(2*np.pi*k/N),
                                     r * np.sin(2*np.pi*k/N), 0])
            supposed_RDF.append(supposed_RDF[-1] + N)
        supposed_RDF = np.array(supposed_RDF[:-1])

        test_set = util.make_raw_query_nlist_test_set(
            box, points, query_points, "ball", r_max, 0, False)
        for ts in test_set:
            rdf = freud.density.RDF(r_max, dr)
            rdf.compute(box, ts[0], query_points, nlist=ts[1])

            npt.assert_allclose(rdf.n_r, supposed_RDF, atol=1e-6)


if __name__ == '__main__':
    unittest.main()
