import unittest
import jpy

jpy.create_jvm(options=['-Djava.class.path=target/test-classes', '-Xmx512M'])


class TestJavaArrays(unittest.TestCase):

    def test_diag_flags_constants(self):
        self.assertIsNotNone(jpy.diag_flags)
        self.assertEqual(jpy.diag_flags.off,  0x00)
        self.assertEqual(jpy.diag_flags.type, 0x01)
        self.assertEqual(jpy.diag_flags.meth, 0x02)
        self.assertEqual(jpy.diag_flags.exec, 0x04)
        self.assertEqual(jpy.diag_flags.mem,  0x08)
        self.assertEqual(jpy.diag_flags.all,  0xff)


    def test_diag_flags_value(self):
        self.assertIsNotNone(jpy.diag_flags)
        self.assertEqual(jpy.diag_flags.value,  0)
        jpy.diag_flags.value = 1
        self.assertEqual(jpy.diag_flags.value, 1)
        jpy.diag_flags.value = 0
        self.assertEqual(jpy.diag_flags.value, 0)
        jpy.diag_flags.value = jpy.diag_flags.exec + jpy.diag_flags.mem
        self.assertEqual(jpy.diag_flags.value, 12)
        jpy.diag_flags.value = 0
        self.assertEqual(jpy.diag_flags.value, 0)
        jpy.diag_flags.value += jpy.diag_flags.exec
        jpy.diag_flags.value += jpy.diag_flags.mem
        self.assertEqual(jpy.diag_flags.value, 12)



if __name__ == '__main__':
    print('\nRunning', __file__)
    unittest.main()
