import unittest
import jpy

jpy.create_jvm(options=['-Djava.class.path=target/test-classes', '-Xmx512M'])

class TestMethodReturnValues(unittest.TestCase):

    def setUp(self):
        self.Dummy = jpy.get_jtype('org.jpy.dummies.MethodReturnValueTestDummy')
        self.Thing = jpy.get_jtype('org.jpy.dummies.Thing')
        self.assertTrue('org.jpy.dummies.MethodReturnValueTestDummy' in jpy.jtypes)

    def test_void(self):
        dummy = self.Dummy()
        self.assertEqual(dummy.getVoid(), None)

    def test_primitive_values(self):
        dummy = self.Dummy()
        self.assertEqual(dummy.getValue_boolean(True), True)
        self.assertEqual(dummy.getValue_byte(11), 11)
        self.assertEqual(dummy.getValue_short(12), 12)
        self.assertEqual(dummy.getValue_int(13), 13)
        self.assertEqual(dummy.getValue_long(14), 14)
        self.assertEqual(round(dummy.getValue_float(15.1), 5), 15.1)   # rounding problem
        self.assertEqual(dummy.getValue_double(16.2), 16.2)

    def test_objects(self):
        dummy = self.Dummy()
        obj = self.Thing()
        self.assertEqual(dummy.getString('Hi!'), 'Hi!')
        self.assertEqual(dummy.getObject(obj), obj)

    def test_array1d_boolean(self):
        dummy = self.Dummy()
        array = dummy.getArray1D_boolean(True, False, True)
        self.assertEqual(len(array), 3)
        self.assertEqual(array[0], True)
        self.assertEqual(array[1], False)
        self.assertEqual(array[2], True)

    def test_array1d_byte(self):
        dummy = self.Dummy()
        array = dummy.getArray1D_byte(-10, 20, 30)
        self.assertEqual(len(array), 3)
        self.assertEqual(array[0], -10)
        self.assertEqual(array[1], 20)
        self.assertEqual(array[2], 30)

    def test_array1d_short(self):
        dummy = self.Dummy()
        array = dummy.getArray1D_short(-10001, 20001, 30001)
        self.assertEqual(len(array), 3)
        self.assertEqual(array[0], -10001)
        self.assertEqual(array[1], 20001)
        self.assertEqual(array[2], 30001)

    def test_array1d_int(self):
        dummy = self.Dummy()
        array = dummy.getArray1D_int(-100001, 200001, 300001)
        self.assertEqual(len(array), 3)
        self.assertEqual(array[0], -100001)
        self.assertEqual(array[1], 200001)
        self.assertEqual(array[2], 300001)

    def test_array1d_long(self):
        dummy = self.Dummy()
        array = dummy.getArray1D_long(-10000000001, 20000000001, 30000000001)
        self.assertEqual(len(array), 3)
        self.assertEqual(array[0], -10000000001)
        self.assertEqual(array[1], 20000000001)
        self.assertEqual(array[2], 30000000001)

    def test_array1d_float(self):
        dummy = self.Dummy()
        array = dummy.getArray1D_float(-1.01, 2.01, 3.01)
        self.assertEqual(len(array), 3)
        self.assertEqual(round(array[0], 5), -1.01)
        self.assertEqual(round(array[1], 5), 2.01)
        self.assertEqual(round(array[2], 5), 3.01)

    def test_array1d_double(self):
        dummy = self.Dummy()
        array = dummy.getArray1D_double(-1.01, 2.01, 3.01)
        self.assertEqual(len(array), 3)
        self.assertEqual(array[0], -1.01)
        self.assertEqual(array[1], 2.01)
        self.assertEqual(array[2], 3.01)

    def test_array1d_String(self):
        dummy = self.Dummy()
        array = dummy.getArray1D_String('A', 'B', 'C')
        self.assertEqual(len(array), 3)
        self.assertEqual(array[0], 'A')
        self.assertEqual(array[1], 'B')
        self.assertEqual(array[2], 'C')

    def test_array1d_Object(self):
        dummy = self.Dummy()
        array = dummy.getArray1D_Object(self.Thing(7), self.Thing(8), self.Thing(9))
        self.assertEqual(len(array), 3)
        self.assertEqual(array[0], self.Thing(7))
        self.assertEqual(array[1], self.Thing(8))
        self.assertEqual(array[2], self.Thing(9))

if __name__ == '__main__':
    unittest.main()
