import unittest
import jpy

jpy.create_jvm(options=['-Djava.class.path=target/test-classes', '-Xmx512M'])

class TestMethodReturnValues(unittest.TestCase):

    def setUp(self):
        self.Dummy = jpy.get_jtype('org.jpy.dummies.MethodReturnValueTestDummy')
        self.assertTrue('org.jpy.dummies.MethodReturnValueTestDummy' in jpy.jtypes)

    def test_void(self):
        obj = self.Dummy()
        self.assertEqual(obj.getValue_void(), None)

    def test_primitives(self):
        obj = self.Dummy()
        self.assertEqual(obj.getValue_boolean(True), True)
        self.assertEqual(obj.getValue_byte(11), 11)
        self.assertEqual(obj.getValue_short(12), 12)
        self.assertEqual(obj.getValue_int(13), 13)
        self.assertEqual(obj.getValue_long(14), 14)
        self.assertEqual(round(obj.getValue_float(15.1), 5), 15.1)   # rounding problem
        self.assertEqual(obj.getValue_double(16.2), 16.2)

    def test_objects(self):
        obj = self.Dummy()
        self.assertEqual(obj.getValue_String('Hi!'), 'Hi!')
        self.assertEqual(obj.getValue_Object(obj), obj)

    # todo - add the others here


if __name__ == '__main__':
    unittest.main()
