# This file was modified by Illumon.
import unittest

import jpyutil
import string

jpyutil.init_jvm(jvm_maxmem='512M', jvm_classpath=['target/test-classes'])
import jpy


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

    # Checking the exceptions for differences (e.g. in white space) can be a huge pain, this helps)
    def hexdump(self, s):
        for i in range(0, len(s), 32):
            sl = s[i:min(i + 32, len(s))]
            fsl = map(lambda x: x if (x in string.printable and not x in "\n\t\r") else ".", sl)
            print("%08d %s %s %s" % (i, " ".join("{:02x}".format(ord(c)) for c in sl),
                                     ("   ".join(map(lambda x : "", range(32 - len(sl))))), sl))

    def test_VerboseException(self):
        fixture = self.Fixture()

        jpy.VerboseExceptions.enabled = True

        self.assertEqual(fixture.throwNpeIfArgIsNull("123456"), 6)

        with self.assertRaises(RuntimeError) as e:
            fixture.throwNpeIfArgIsNullNested(None)
        actual_message = str(e.exception)
        expected_message = "java.lang.RuntimeException: Nested exception\n\t" \
                           "at org.jpy.fixtures.ExceptionTestFixture." \
                           "throwNpeIfArgIsNullNested(ExceptionTestFixture.java:43)\n" \
                           "caused by java.lang.NullPointerException\n\t" \
                           "at org.jpy.fixtures.ExceptionTestFixture." \
                           "throwNpeIfArgIsNull(ExceptionTestFixture.java:32)\n\t" \
                           "at org.jpy.fixtures.ExceptionTestFixture." \
                           "throwNpeIfArgIsNullNested(ExceptionTestFixture.java:41)\n"

        # self.hexdump(actual_message)
        # self.hexdump(expected_message)
        # print [i for i in xrange(min(len(expected_message), len(actual_message))) if actual_message[i] != expected_message[i]]

        self.assertEquals(actual_message, expected_message)

        with self.assertRaises(RuntimeError) as e:
            fixture.throwNpeIfArgIsNullNested3(None)
        actual_message = str(e.exception)
        expected_message = "java.lang.RuntimeException: Nested exception 3\n\t" \
                           "at org.jpy.fixtures.ExceptionTestFixture." \
                           "throwNpeIfArgIsNullNested3(ExceptionTestFixture.java:55)\n" \
                           "caused by java.lang.RuntimeException: Nested exception\n\t" \
                           "at org.jpy.fixtures.ExceptionTestFixture." \
                           "throwNpeIfArgIsNullNested(ExceptionTestFixture.java:43)\n\t" \
                           "at org.jpy.fixtures.ExceptionTestFixture." \
                           "throwNpeIfArgIsNullNested2(ExceptionTestFixture.java:48)\n\t" \
                           "at org.jpy.fixtures.ExceptionTestFixture." \
                           "throwNpeIfArgIsNullNested3(ExceptionTestFixture.java:53)\n" \
                           "caused by java.lang.NullPointerException\n\t" \
                           "at org.jpy.fixtures.ExceptionTestFixture." \
                           "throwNpeIfArgIsNull(ExceptionTestFixture.java:32)\n\t" \
                           "at org.jpy.fixtures.ExceptionTestFixture." \
                           "throwNpeIfArgIsNullNested(ExceptionTestFixture.java:41)\n\t... 2 more\n"

        # self.hexdump(actual_message)
        # self.hexdump(expected_message)
        # print [i for i in xrange(min(len(expected_message), len(actual_message))) if actual_message[i] != expected_message[i]]

        self.assertEquals(actual_message, expected_message)

        jpy.VerboseExceptions.enabled = False


if __name__ == '__main__':
    print('\nRunning ' + __file__)
    unittest.main()
