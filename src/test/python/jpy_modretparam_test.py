import unittest
import array
import jpy

debug = False
debug = True
jpy.create_jvm(options=['-Djava.class.path=target/test-classes', '-Xmx512M'], debug=debug)


def annotate_fixture_methods(type, method):
    #print('annotate_fixture_methods: type =', type, ', method =', method.name)
    if method.name == 'modifyThing':
        method.set_param_mutable(0, True)
        method.set_param_return(0, False)
    elif method.name == 'returnThing':
        method.set_param_mutable(0, False)
        method.set_param_return(0, True)
    elif method.name == 'modifyAndReturnThing':
        method.set_param_mutable(0, True)
        method.set_param_return(0, True)
    elif method.name == 'modifyIntArray':
        method.set_param_mutable(0, True)
        method.set_param_return(0, False)
    elif method.name == 'returnIntArray':
        method.set_param_mutable(0, False)
        method.set_param_return(0, True)
    elif method.name == 'modifyAndReturnIntArray':
        method.set_param_mutable(0, True)
        method.set_param_return(0, True)
    return True


jpy.type_callbacks['org.jpy.fixtures.ModifyAndReturnParametersTestFixture'] = annotate_fixture_methods


class TestMutableAndReturnParameters(unittest.TestCase):

    def setUp(self):
        self.Fixture = jpy.get_class('org.jpy.fixtures.ModifyAndReturnParametersTestFixture')
        self.assertIsNotNone(self.Fixture)
        self.Thing = jpy.get_class('org.jpy.fixtures.Thing')
        self.assertIsNotNone(self.Thing)


    def test_modifyThing(self):
        fixture = self.Fixture()
        t = self.Thing()
        self.assertEqual(t.getValue(), 0)
        fixture.modifyThing(t, 11)
        self.assertEqual(t.getValue(), 11)


    def test_returnThing(self):
        fixture = self.Fixture()
        t1 = self.Thing(21)
        t2 = fixture.returnThing(t1)
        self.assertEqual(t1.getValue(), 21)
        self.assertEqual(t2.getValue(), 21)
        self.assertEqual(t1, t2)
        self.assertTrue(t1 is t2)


    def test_modifyAndReturnThing(self):
        fixture = self.Fixture()
        t1 = self.Thing()
        t2 = fixture.modifyAndReturnThing(t1, 31)
        self.assertEqual(t1.getValue(), 31)
        self.assertEqual(t2.getValue(), 31)
        self.assertEqual(t1, t2)
        self.assertTrue(t1 is t2)


    def test_modifyIntArray(self):
        fixture = self.Fixture()

        fixture.modifyIntArray(None, 12, 13, 14)
        """
        a = jpy.CArray('i', 3)
        fixture.modifyIntArray(a, 12, 13, 14)
        self.assertEqual(a[0], 12)
        self.assertEqual(a[1], 13)
        self.assertEqual(a[2], 14)

        a = array.array('i', [0, 0, 0])
        fixture.modifyIntArray(a, 12, 13, 14)
        self.assertEqual(a[0], 12)
        self.assertEqual(a[1], 13)
        self.assertEqual(a[2], 14)

        a = jpy.array('int', 3)
        fixture.modifyIntArray(a, 12, 13, 14)
        self.assertEqual(a[0], 12)
        self.assertEqual(a[1], 13)
        self.assertEqual(a[2], 14)
        """

    """
    def test_returnIntArray(self):
        fixture = self.Fixture()

        a1 = None
        a2 = fixture.returnIntArray(a1)
        self.assertEqual(a1 is a2)

        a1 = jpy.CArray('i', 3)
        a2 = fixture.returnIntArray(a1)
        self.assertEqual(a1 is a2)

        a1 = array.array('i', [0, 0, 0])
        a2 = fixture.returnIntArray(a1)
        self.assertEqual(a1 is a2)

        a1 = jpy.array('int', 3)
        a2 = fixture.returnIntArray(a1)
        self.assertEqual(a1 is a2)
    """


if __name__ == '__main__':
    unittest.main()
