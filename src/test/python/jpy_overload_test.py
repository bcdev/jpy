import unittest
import jpy

#debug = True
debug = False
jpy.create_jvm(options=['-Djava.class.path=target/test-classes', '-Xmx512M'], debug=debug)


class TestConstructorOverloads(unittest.TestCase):


    def setUp(self):
        self.Fixture = jpy.get_class('org.jpy.fixtures.ConstructorOverloadTestFixture')
        self.assertIsNotNone(self.Fixture)


    def test_FloatConstructors(self):
        fixture = self.Fixture()
        self.assertEqual(fixture.getState(), '')

        fixture = self.Fixture(12)
        self.assertEqual(fixture.getState(), 'Integer(12)')

        fixture = self.Fixture(12, 34)
        self.assertEqual(fixture.getState(), 'Integer(12),Integer(34)')

        fixture = self.Fixture(0.12)
        self.assertEqual(fixture.getState(), 'Float(0.12)')

        fixture = self.Fixture(0.12, 0.34)
        self.assertEqual(fixture.getState(), 'Float(0.12),Float(0.34)')

        fixture = self.Fixture(0.12, 34)
        self.assertEqual(fixture.getState(), 'Float(0.12),Integer(34)')

        fixture = self.Fixture(12, 0.34)
        self.assertEqual(fixture.getState(), 'Integer(12),Float(0.34)')

        with  self.assertRaises(RuntimeError, msg='RuntimeError expected') as e:
            fixture = self.Fixture(12, '34')
        self.assertEqual(str(e.exception), 'no matching Java method overloads found')


class TestMethodOverloads(unittest.TestCase):


    def setUp(self):
        self.Fixture = jpy.get_class('org.jpy.fixtures.MethodOverloadTestFixture')
        self.assertIsNotNone(self.Fixture)


    def test_2ArgOverloadsWithVaryingTypes(self):
        fixture = self.Fixture()

        self.assertEqual(fixture.join(12, 32), 'Integer(12),Integer(32)')
        self.assertEqual(fixture.join(12, 3.2), 'Integer(12),Double(3.2)')
        self.assertEqual(fixture.join(12, 'abc'), 'Integer(12),String(abc)')
        self.assertEqual(fixture.join(1.2, 32), 'Double(1.2),Integer(32)')
        self.assertEqual(fixture.join(1.2, 3.2), 'Double(1.2),Double(3.2)')
        self.assertEqual(fixture.join(1.2, 'abc'), 'Double(1.2),String(abc)')
        self.assertEqual(fixture.join('efg', 32), 'String(efg),Integer(32)')
        self.assertEqual(fixture.join('efg', 3.2), 'String(efg),Double(3.2)')
        self.assertEqual(fixture.join('efg', 'abc'), 'String(efg),String(abc)')

        with  self.assertRaises(RuntimeError, msg='RuntimeError expected') as e:
            fixture.join(object(), 32)
        self.assertEqual(str(e.exception), 'no matching Java method overloads found')


    def test_nArgOverloads(self):
        fixture = self.Fixture()

        self.assertEqual(fixture.join('x'), 'String(x)')
        self.assertEqual(fixture.join('x', 'y'), 'String(x),String(y)')
        self.assertEqual(fixture.join('x', 'y', 'z'), 'String(x),String(y),String(z)')

        with  self.assertRaises(RuntimeError, msg='RuntimeError expected') as e:
            fixture.join('x', 'y', 'z', 'u')
        self.assertEqual(str(e.exception), 'no matching Java method overloads found')


    def test_nArgOverloadsAreFoundInBaseClass(self):
        Fixture = jpy.get_class('org.jpy.fixtures.MethodOverloadTestFixture$MethodOverloadTestFixture2')
        fixture = Fixture()

        self.assertEqual(fixture.join('x'), 'String(x)')
        self.assertEqual(fixture.join('x', 'y'), 'String(x),String(y)')
        self.assertEqual(fixture.join('x', 'y', 'z'), 'String(x),String(y),String(z)')
        self.assertEqual(fixture.join('x', 'y', 'z', 'u'), 'String(x),String(y),String(z),String(u)')

        with  self.assertRaises(RuntimeError, msg='RuntimeError expected') as e:
            fixture.join('x', 'y', 'z', 'u', 'v')
        self.assertEqual(str(e.exception), 'no matching Java method overloads found')



if __name__ == '__main__':
    unittest.main()
