import unittest
import jpy

#debug = True
debug = False
jpy.create_jvm(options=['-Djava.class.path=target/test-classes', '-Xmx512M'], debug=debug)

class TestJavaArrays(unittest.TestCase):

    def do_test_array_protocol(self, type, initial, expected):
        a = jpy.array(type, 3)
        self.assertEqual(len(a), 3)
        self.assertEqual(a[0], initial[0])
        self.assertEqual(a[1], initial[1])
        self.assertEqual(a[2], initial[2])
        a[0] = expected[0]
        a[1] = expected[1]
        a[2] = expected[2]
        self.assertEqual(a[0], expected[0])
        self.assertEqual(a[1], expected[1])
        self.assertEqual(a[2], expected[2])


    def do_test_array_protocol_float(self, type, initial, expected, places):
        a = jpy.array(type, 3)
        self.assertEqual(len(a), 3)
        self.assertEqual(a[0], initial[0])
        self.assertEqual(a[1], initial[1])
        self.assertEqual(a[2], initial[2])
        a[0] = expected[0]
        a[1] = expected[1]
        a[2] = expected[2]
        self.assertAlmostEqual(a[0], expected[0], places=places)
        self.assertAlmostEqual(a[1], expected[1], places=places)
        self.assertAlmostEqual(a[2], expected[2], places=places)


    def test_array_boolean(self):
        self.do_test_array_protocol('boolean', [False, False, False], [True, False, True])


    def test_array_char(self):
        self.do_test_array_protocol('char', [0, 0, 0], [0, 100, 32767])


    def test_array_byte(self):
        self.do_test_array_protocol('byte', [0, 0, 0], [-128, 100, 127])


    def test_array_short(self):
        self.do_test_array_protocol('short', [0, 0, 0], [-32768, 100, 32767])


    def test_array_int(self):
        self.do_test_array_protocol('int', [0, 0, 0], [-2147483648, 100, 2147483647])


    def test_array_long(self):
        self.do_test_array_protocol('long', [0, 0, 0], [-9223372036854775808, 100, 9223372036854775807])


    def test_array_float(self):
        self.do_test_array_protocol_float('float', [0, 0, 0], [-1.001, 0.001, 1.001], places=5)


    def test_array_double(self):
        self.do_test_array_protocol_float('double', [0, 0, 0], [-1.001, 0.001, 1.001], places=10)


    def test_array_object(self):
        self.do_test_array_protocol('java.lang.String', [None, None, None], ['A', 'B', 'C'])
        F = jpy.get_class('java.io.File')
        self.do_test_array_protocol('java.io.File', [None, None, None], [F('A'), F('B'), F('C')])


if __name__ == '__main__':
    print('\nRunning', __file__)
    unittest.main()
