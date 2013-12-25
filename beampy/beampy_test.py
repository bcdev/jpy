import unittest
import beampy

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
        import array
        a = array.array('f', w * [0])
        b.readPixels(0, 0, w, 1, a)
        self.assertAlmostEqual(a[100], 232.93376, places=5)
        self.assertAlmostEqual(a[101], 232.57832, places=5)
        self.assertAlmostEqual(a[102], 226.83096, places=5)
        self.assertAlmostEqual(a[103], 226.83096, places=5)
        self.assertAlmostEqual(a[104], 219.54100, places=5)


    def test_readPixels_with_numpy_array(self):
        import numpy as np
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



if __name__ == '__main__':
    unittest.main()
