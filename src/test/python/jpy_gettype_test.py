import unittest
import jpy
import sys

jpy.create_jvm(options=['-Djava.class.path=target/test-classes', '-Xmx512M'])

if sys.version_info >= (3,0,0):
    TYPE_STR_PREFIX = '<class '
else:
    TYPE_STR_PREFIX = '<type '

class TestGetClass(unittest.TestCase):

    def test_get_class_of_primitive_array(self):
        IntArray1D = jpy.get_type('[I')
        self.assertEqual(str(IntArray1D), TYPE_STR_PREFIX + "'[I'>")

        IntArray2D = jpy.get_type('[[I')
        self.assertEqual(str(IntArray2D), TYPE_STR_PREFIX + "'[[I'>")

        IntArray3D = jpy.get_type('[[[I')
        self.assertEqual(str(IntArray3D), TYPE_STR_PREFIX + "'[[[I'>")

        with self.assertRaises(RuntimeError) as e:
            IntArray1D()
        self.assertEqual(str(e.exception), "no constructor found (missing JType attribute '__jinit__')")


    def test_get_class_of_object_array(self):
        StringArray1D = jpy.get_type('[Ljava.lang.String;')
        self.assertEqual(str(StringArray1D), TYPE_STR_PREFIX + "'[Ljava.lang.String;'>")

        StringArray2D = jpy.get_type('[[Ljava.lang.String;')
        self.assertEqual(str(StringArray2D), TYPE_STR_PREFIX + "'[[Ljava.lang.String;'>")

        StringArray3D = jpy.get_type('[[[Ljava.lang.String;')
        self.assertEqual(str(StringArray3D), TYPE_STR_PREFIX + "'[[[Ljava.lang.String;'>")

        with self.assertRaises(RuntimeError) as e:
            StringArray1D()
        self.assertEqual(str(e.exception), "no constructor found (missing JType attribute '__jinit__')")


    def test_get_class_of_unknown_type(self):
        with  self.assertRaises(ValueError) as e:
            String = jpy.get_type('java.lang.Spring')
        self.assertEqual(str(e.exception), "Java class 'java.lang.Spring' not found")

        with  self.assertRaises(ValueError) as e:
            IntArray = jpy.get_type('int[]')
        self.assertEqual(str(e.exception), "Java class 'int[]' not found")

if __name__ == '__main__':
    print('\nRunning', __file__)
    unittest.main()
