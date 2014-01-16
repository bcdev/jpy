import unittest
import jpy
import numpy as np

#debug = True
debug = False
jpy.create_jvm(options=['-Djava.class.path=target/test-classes', '-Xmx512M'], debug=debug)


class TestTypeResolution(unittest.TestCase):

    def setUp(self):
        self.Fixture = jpy.get_class('org.jpy.fixtures.TypeResolutionTestFixture')
        self.assertIsNotNone(self.Fixture)


    def test_ThatTypeIsResolvedLate(self):

        self.assertTrue('org.jpy.fixtures.TypeResolutionTestFixture' in jpy.types)
        self.assertTrue('org.jpy.fixtures.TypeResolutionTestFixture$SuperThing' in jpy.types)
        self.assertTrue('org.jpy.fixtures.Thing' in jpy.types)

        fixture = self.Fixture()

        # Create a thing instance, the type 'org.jpy.fixtures.Thing' is not resolved yet
        thing = fixture.createSuperThing(2001)

        # Assert that 'org.jpy.fixtures.Thing' is not resolved yet
        SuperThing = jpy.types['org.jpy.fixtures.TypeResolutionTestFixture$SuperThing']
        self.assertFalse('add' in SuperThing.__dict__)

        # Assert that 'org.jpy.fixtures.Thing' is not resolved yet
        Thing = jpy.types['org.jpy.fixtures.Thing']
        self.assertFalse('getValue' in Thing.__dict__)

        # Calling 'add()' on SuperThing will resolve 'org.jpy.fixtures.Thing'
        thing.add(10)
        self.assertTrue('add' in SuperThing.__dict__)
        self.assertTrue('getValue' in Thing.__dict__)

        value = thing.getValue()
        self.assertEqual(value, 2011)




if __name__ == '__main__':
    unittest.main()
