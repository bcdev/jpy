import unittest
import jpyutil
jpyutil.preload_jvm_dll()
import jpy


jpy.create_jvm(options=['-Djava.class.path=target/test-classes', '-Xmx512M'])

class TestExceptions(unittest.TestCase):


    def setUp(self):
        self.Fixture = jpy.get_type('org.jpy.fixtures.ExceptionTestFixture')
        self.assertIsNotNone(self.Fixture)


    def test_NullPointerException(self):
        fixture = self.Fixture()

        self.assertEqual(fixture.throwNpeIfArgIsNull("123456"), 6)

        with  self.assertRaises(RuntimeError, msg='Java NullPointerException expected') as e:
            fixture.throwNpeIfArgIsNull(None)
        self.assertEqual(str(e.exception), 'java.lang.NullPointerException')


    def test_ArrayIndexOutOfBoundsException(self):
        fixture = self.Fixture()
        self.assertEqual(fixture.throwAioobeIfIndexIsNotZero(0), 101)

        with  self.assertRaises(RuntimeError, msg='Java ArrayIndexOutOfBoundsException expected') as e:
            fixture.throwAioobeIfIndexIsNotZero(1)
        self.assertEqual(str(e.exception), 'java.lang.ArrayIndexOutOfBoundsException: 1')

        with  self.assertRaises(RuntimeError, msg='Java ArrayIndexOutOfBoundsException expected') as e:
            fixture.throwAioobeIfIndexIsNotZero(-1)
        self.assertEqual(str(e.exception), 'java.lang.ArrayIndexOutOfBoundsException: -1')


    def test_RuntimeException(self):
        fixture = self.Fixture()
        fixture.throwRteIfMessageIsNotNull(None)

        with  self.assertRaises(RuntimeError, msg='Java RuntimeException expected') as e:
            fixture.throwRteIfMessageIsNotNull("Evil!")
        self.assertEqual(str(e.exception), 'java.lang.RuntimeException: Evil!')


    def test_IOException(self):
        fixture = self.Fixture()
        fixture.throwIoeIfMessageIsNotNull(None)

        with  self.assertRaises(RuntimeError, msg='Java IOException expected') as e:
            fixture.throwIoeIfMessageIsNotNull("Evil!")
        self.assertEqual(str(e.exception), 'java.io.IOException: Evil!')


if __name__ == '__main__':
    print('\nRunning ' + __file__)
    unittest.main()
