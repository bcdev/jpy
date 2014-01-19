import unittest
import jpy

jpy.create_jvm(options=['-Djava.class.path=target/test-classes', '-Xmx512M'])

class TestGetClass(unittest.TestCase):

    def test_get_class_of_primitive_array(self):
        IntArray1D = jpy.get_class('[I')
        self.assertEqual(str(IntArray1D), "<class '[I'>")

        IntArray2D = jpy.get_class('[[I')
        self.assertEqual(str(IntArray2D), "<class '[[I'>")

        IntArray3D = jpy.get_class('[[[I')
        self.assertEqual(str(IntArray3D), "<class '[[[I'>")

        # todo - uncomment and find Python crash here. fix me!
        #with  self.assertRaises(RuntimeError) as e:
        #    IntArray1D()
        #self.assertEqual(str(e.exception), 'no matching Java method overloads found')


    def test_get_class_of_object_array(self):
        StringArray1D = jpy.get_class('[Ljava.lang.String;')
        self.assertEqual(str(StringArray1D), "<class '[Ljava.lang.String;'>")

        StringArray2D = jpy.get_class('[[Ljava.lang.String;')
        self.assertEqual(str(StringArray2D), "<class '[[Ljava.lang.String;'>")

        StringArray3D = jpy.get_class('[[[Ljava.lang.String;')
        self.assertEqual(str(StringArray3D), "<class '[[[Ljava.lang.String;'>")

        # todo - uncomment and find Python crash here. fix me!
        #with  self.assertRaises(RuntimeError) as e:
        #    StringArray1D()
        #self.assertEqual(str(e.exception), 'no matching Java method overloads found')


    def test_get_class_of_unknown_type(self):
        with  self.assertRaises(ValueError) as e:
            String = jpy.get_class('java.lang.Spring')
        self.assertEqual(str(e.exception), 'Java class not found')



if __name__ == '__main__':
    print('\nRunning', __file__)
    unittest.main()
