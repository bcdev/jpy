import unittest
import beampy

class TestBeamIO(unittest.TestCase):


    def setUp(self):
        self.product = beampy.ProductIO.readProduct('../beam-extapi/TEST.N1')
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


    def test_readPixels(self):
        w = self.product.getSceneRasterWidth()
        h = self.product.getSceneRasterHeight()
        b = self.product.getBand('radiance_13')
        data = b.readPixels(0, 0, w, h, None)


if __name__ == '__main__':
    unittest.main()
