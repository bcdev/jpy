import unittest
import jpy

#debug = True
debug = False
jpy.create_jvm(options=['-Djava.class.path=target/test-classes', '-Xmx512M'], debug=debug)

class TestExceptions(unittest.TestCase):


    def setUp(self):
        self.Fixture = jpy.get_class('org.jpy.fixtures.MethodOverloadTestFixture')
        self.assertTrue('org.jpy.fixtures.MethodOverloadTestFixture' in jpy.types)


    def test_2ArgOverloadsWithVaryingTypes(self):
        fixture = self.Fixture()

        self.assertEqual(fixture.join(12, 32), 'I12 I32')
        self.assertEqual(fixture.join(12, 3.2), 'I12 D3.2')
        self.assertEqual(fixture.join(12, 'abc'), 'I12 Sabc')
        self.assertEqual(fixture.join(1.2, 32), 'D1.2 I32')
        self.assertEqual(fixture.join(1.2, 3.2), 'D1.2 D3.2')
        self.assertEqual(fixture.join(1.2, 'abc'), 'D1.2 Sabc')
        self.assertEqual(fixture.join('efg', 32), 'Sefg I32')
        self.assertEqual(fixture.join('efg', 3.2), 'Sefg D3.2')
        self.assertEqual(fixture.join('efg', 'abc'), 'Sefg Sabc')

        with  self.assertRaises(RuntimeError, msg='RuntimeError expected') as e:
            fixture.join(object(), 32)
        self.assertEqual(str(e.exception), 'no matching Java method overloads found')


    def test_nArgOverloads(self):
        fixture = self.Fixture()

        self.assertEqual(fixture.join('x'), 'Sx')
        self.assertEqual(fixture.join('x', 'y'), 'Sx Sy')
        self.assertEqual(fixture.join('x', 'y', 'z'), 'Sx Sy Sz')

        with  self.assertRaises(RuntimeError, msg='RuntimeError expected') as e:
            fixture.join('x', 'y', 'z', 'u')
        self.assertEqual(str(e.exception), 'no matching Java method overloads found')


    def test_nArgOverloadsAreFoundInBaseClass(self):
        Fixture = jpy.get_class('org.jpy.fixtures.MethodOverloadTestFixture$MethodOverloadTestFixture2')
        fixture = Fixture()

        self.assertEqual(fixture.join('x'), 'Sx')
        self.assertEqual(fixture.join('x', 'y'), 'Sx Sy')
        self.assertEqual(fixture.join('x', 'y', 'z'), 'Sx Sy Sz')
        self.assertEqual(fixture.join('x', 'y', 'z', 'u'), 'Sx Sy Sz Su')

        with  self.assertRaises(RuntimeError, msg='RuntimeError expected') as e:
            fixture.join('x', 'y', 'z', 'u', 'v')
        self.assertEqual(str(e.exception), 'no matching Java method overloads found')



if __name__ == '__main__':
    unittest.main()
