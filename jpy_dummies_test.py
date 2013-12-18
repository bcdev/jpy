import unittest
import jpy

#debug = True
debug = False
jpy.create_jvm(options=['-Djava.class.path=target/test-classes', '-Xmx512M'], debug=debug)

class TestMethodReturnValues(unittest.TestCase):


    def setUp(self):
        self.Dummy = jpy.get_class('org.jpy.dummies.MethodReturnValueTestDummy')
        self.Thing = jpy.get_class('org.jpy.dummies.Thing')
        self.assertTrue('org.jpy.dummies.MethodReturnValueTestDummy' in jpy.types)


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
        self.assertAlmostEqual(dummy.getValue_float(15.1), 15.1, places=5)
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
        self.assertAlmostEqual(array[0], -1.01, places=5)
        self.assertAlmostEqual(array[1], 2.01, places=5)
        self.assertAlmostEqual(array[2], 3.01, places=5)


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



class TestFields(unittest.TestCase):


    def setUp(self):
        self.Dummy = jpy.get_class('org.jpy.dummies.FieldTestDummy')
        self.Thing = jpy.get_class('org.jpy.dummies.Thing')
        self.String = jpy.get_class('java.lang.String')
        self.assertTrue('org.jpy.dummies.FieldTestDummy' in jpy.types)
        self.assertTrue('org.jpy.dummies.Thing' in jpy.types)


    def test_static_fields(self):
        self.assertEqual(self.Dummy.z_STATIC_FIELD, True)
        self.assertEqual(self.Dummy.b_STATIC_FIELD, 123)
        self.assertEqual(self.Dummy.c_STATIC_FIELD, 65)
        self.assertEqual(self.Dummy.s_STATIC_FIELD, 12345)
        self.assertEqual(self.Dummy.i_STATIC_FIELD, 123456789)
        self.assertEqual(self.Dummy.j_STATIC_FIELD, 1234567890123456789)
        self.assertAlmostEqual(self.Dummy.f_STATIC_FIELD, 0.12345, places=5)
        self.assertAlmostEqual(self.Dummy.d_STATIC_FIELD, 0.123456789)

        self.assertEqual(self.Dummy.S_OBJ_STATIC_FIELD, 'ABC')
        self.assertEqual(self.Dummy.l_OBJ_STATIC_FIELD, self.Thing(123))


    def test_primitive_instance_fields(self):
        dummy = self.Dummy()
        self.assertEqual(dummy.zInstField, False)
        self.assertEqual(dummy.bInstField, 0)
        self.assertEqual(dummy.cInstField, 0)
        self.assertEqual(dummy.sInstField, 0)
        self.assertEqual(dummy.iInstField, 0)
        self.assertEqual(dummy.jInstField, 0)
        self.assertEqual(dummy.fInstField, 0)
        self.assertEqual(dummy.dInstField, 0)

        dummy.zInstField = True
        dummy.bInstField = 123
        dummy.cInstField = 65
        dummy.sInstField = 12345
        dummy.iInstField = 123456789
        dummy.jInstField = 1234567890123456789
        dummy.fInstField = 0.12345
        dummy.dInstField = 0.123456789

        self.assertEqual(dummy.zInstField, True)
        self.assertEqual(dummy.bInstField, 123)
        self.assertEqual(dummy.cInstField, 65)
        self.assertEqual(dummy.sInstField, 12345)
        self.assertEqual(dummy.iInstField, 123456789)
        self.assertEqual(dummy.jInstField, 1234567890123456789)
        self.assertAlmostEqual(dummy.fInstField, 0.12345, places=5)
        self.assertAlmostEqual(dummy.dInstField, 0.123456789)

    def test_object_instance_fields(self):
        dummy = self.Dummy()
        self.assertEqual(dummy.zObjInstField, None)
        self.assertEqual(dummy.bObjInstField, None)
        self.assertEqual(dummy.cObjInstField, None)
        self.assertEqual(dummy.sObjInstField, None)
        self.assertEqual(dummy.iObjInstField, None)
        self.assertEqual(dummy.jObjInstField, None)
        self.assertEqual(dummy.fObjInstField, None)
        self.assertEqual(dummy.dObjInstField, None)
        self.assertEqual(dummy.SObjInstField, None)
        self.assertEqual(dummy.lObjInstField, None)

        dummy.zObjInstField = True
        dummy.bObjInstField = 123
        dummy.cObjInstField = 65
        dummy.sObjInstField = 12345
        dummy.iObjInstField = 123456789
        dummy.jObjInstField = 1234567890123456789
        dummy.fObjInstField = 0.12345
        dummy.dObjInstField = 0.123456789
        dummy.SObjInstField = 'ABC'
        dummy.lObjInstField = self.Thing(123)

        self.assertEqual(dummy.zObjInstField, True)
        self.assertEqual(dummy.bObjInstField, 123)
        self.assertEqual(dummy.cObjInstField, 65)
        self.assertEqual(dummy.sObjInstField, 12345)
        self.assertEqual(dummy.iObjInstField, 123456789)
        self.assertEqual(dummy.jObjInstField, 1234567890123456789)
        self.assertAlmostEqual(dummy.fObjInstField, 0.12345, places=5)
        self.assertAlmostEqual(dummy.dObjInstField, 0.123456789)
        self.assertEqual(dummy.SObjInstField, 'ABC')
        self.assertEqual(dummy.lObjInstField, self.Thing(123))

if __name__ == '__main__':
    unittest.main()
