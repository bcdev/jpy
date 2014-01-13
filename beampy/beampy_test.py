import unittest
import beampy
import numpy as np
import array

class TestBeamIO(unittest.TestCase):

    def setUp(self):
        self.product = beampy.ProductIO.readProduct('../../beam-extapi/TEST.N1')
        self.assertIsNotNone(self.product)


    def tearDown(self):
        self.product.dispose()


    def test_getBandNames(self):
        names = self.product.getBandNames()
        self.assertEqual(len(names), 17)
        self.assertEqual(names[0], 'radiance_1')
        self.assertEqual(names[14], 'radiance_15')
        self.assertEqual(names[15], 'l1_flags')
        self.assertEqual(names[16], 'detector_index')


    def test_getSceneRasterWidthAndHeight(self):
        w = self.product.getSceneRasterWidth()
        h = self.product.getSceneRasterHeight()
        self.assertEqual(w, 1121)
        self.assertEqual(h,  881)


    def test_readPixels_performance(self):
        w = self.product.getSceneRasterWidth()
        h = self.product.getSceneRasterHeight()
        b = self.product.getBand('radiance_13')
        a = np.zeros(w, dtype=np.float32)

        import time

        t0 = time.time()
        for y in range(h):
            b.readPixels(0, 0, w, 1, a)
        t1 = time.time()

        dt = t1 - t0
        print('Band.readPixels(): w =', w, ', dtype=np.float32:', h, 'calls in', dt*1000, 'ms, that is ', dt*1000/y, 'ms per call')





    def test_readPixels_with_java_array(self):
        w = self.product.getSceneRasterWidth()
        h = self.product.getSceneRasterHeight()
        b = self.product.getBand('radiance_13')
        a = beampy.jpy.array('float', w)
        b.readPixels(0, 0, w, 1, a)
        self.assertAlmostEqual(a[100], 232.93376, places=5)
        self.assertAlmostEqual(a[101], 232.57832, places=5)
        self.assertAlmostEqual(a[102], 226.83096, places=5)
        self.assertAlmostEqual(a[103], 226.83096, places=5)
        self.assertAlmostEqual(a[104], 219.54100, places=5)


    def test_readPixels_with_python_array(self):
        w = self.product.getSceneRasterWidth()
        h = self.product.getSceneRasterHeight()
        b = self.product.getBand('radiance_13')
        a = array.array('f', w * [0])
        b.readPixels(0, 0, w, 1, a)
        self.assertAlmostEqual(a[100], 232.93376, places=5)
        self.assertAlmostEqual(a[101], 232.57832, places=5)
        self.assertAlmostEqual(a[102], 226.83096, places=5)
        self.assertAlmostEqual(a[103], 226.83096, places=5)
        self.assertAlmostEqual(a[104], 219.54100, places=5)


    def test_readPixels_with_numpy_array(self):
        w = self.product.getSceneRasterWidth()
        h = self.product.getSceneRasterHeight()
        b = self.product.getBand('radiance_13')
        a = np.zeros(w, dtype=np.float32)
        b.readPixels(0, 0, w, 1, a)
        self.assertAlmostEqual(a[100], 232.93376, places=5)
        self.assertAlmostEqual(a[101], 232.57832, places=5)
        self.assertAlmostEqual(a[102], 226.83096, places=5)
        self.assertAlmostEqual(a[103], 226.83096, places=5)
        self.assertAlmostEqual(a[104], 219.54100, places=5)


    def test_readPixels_with_carray(self):
        w = self.product.getSceneRasterWidth()
        h = self.product.getSceneRasterHeight()
        b = self.product.getBand('radiance_13')
        a = beampy.jpy.CArray('f', w)
        b.readPixels(0, 0, w, 1, a)
        self.assertAlmostEqual(a[100], 232.93376, places=5)
        self.assertAlmostEqual(a[101], 232.57832, places=5)
        self.assertAlmostEqual(a[102], 226.83096, places=5)
        self.assertAlmostEqual(a[103], 226.83096, places=5)
        self.assertAlmostEqual(a[104], 219.54100, places=5)


    def test_readValidMask_with_numpy_array(self):
        w = self.product.getSceneRasterWidth()
        h = self.product.getSceneRasterHeight()
        b = self.product.getBand('radiance_13')
        a = np.zeros(w, dtype=np.bool)
        b.readValidMask(0, 0, w, 1, a)
        self.assertEqual(a[0], 0)
        self.assertEqual(a[1], 0)
        self.assertEqual(a[2], 0)
        self.assertEqual(a[100], 1)
        self.assertEqual(a[101], 1)
        self.assertEqual(a[102], 1)
        self.assertEqual(a[103], 1)
        self.assertEqual(a[104], 1)


if __name__ == '__main__':
    unittest.main()
