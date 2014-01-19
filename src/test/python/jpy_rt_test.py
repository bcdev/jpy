import unittest
import jpy

#jpy.diag_flags.value = 0xff
jpy.create_jvm(options=['-Xmx512M'])

class TestString(unittest.TestCase):


    def setUp(self):
        self.String = jpy.get_class('java.lang.String')
        self.assertIsNotNone(self.String)


    def test_constructor(self):
        s = self.String('Bibo')
        self.assertEqual(type(s), self.String)
        self.assertEqual(str(s), 'Bibo')


    def test_toString(self):
        s = self.String('Bibo')
        self.assertTrue('toString' in self.String.__dict__)
        s = s.toString()
        self.assertEqual(s, 'Bibo')


    def test_substring(self):
        s = self.String('Bibo')
        self.assertTrue('substring' in self.String.__dict__)
        s2 = s.substring(0, 2)
        self.assertEqual(s2, 'Bi')
        s2 = s.substring(2)
        self.assertEqual(s2, 'bo')


    def test_split(self):
        s = self.String('/usr/local/bibo')
        self.assertTrue('split' in self.String.__dict__)
        array = s.split('/')
        self.assertEqual(len(array), 4)
        self.assertEqual(array[0], '')
        self.assertEqual(array[1], 'usr')
        self.assertEqual(array[2], 'local')
        self.assertEqual(array[3], 'bibo')
        array = s.split('/', 2)
        self.assertEqual(array[0], '')
        self.assertEqual(array[1], 'usr/local/bibo')


    def test_getBytes(self):
        s = self.String('Bibo')
        self.assertTrue('getBytes' in self.String.__dict__)
        array = s.getBytes()
        self.assertEqual(str(type(array)), "<class '[B'>")
        self.assertEqual(len(array), 4)
        self.assertEqual(array[0], 66)
        self.assertEqual(array[1], 105)
        self.assertEqual(array[2], 98)
        self.assertEqual(array[3], 111)


    def test_getClass(self):
        s = self.String()
        c = s.getClass()
        self.assertEqual('java.lang.String', c.getName())


import os


class TestFile(unittest.TestCase):


    def setUp(self):
        self.File = jpy.get_class('java.io.File')
        self.assertIsNotNone(self.File)


    def test_constructor(self):
        f = self.File('/usr/local/bibo')
        self.assertEqual(type(f), self.File)
        self.assertEqual(str(f).split(os.sep), ['', 'usr', 'local', 'bibo'])


    def test_getPath(self):
        f = self.File('/usr/local/bibo')
        self.assertTrue('getPath' in self.File.__dict__)
        path = f.getPath()
        self.assertEqual(path.split(os.sep), ['', 'usr', 'local', 'bibo'])


    def test_getName(self):
        f = self.File('/usr/local/bibo')
        self.assertTrue('getName' in self.File.__dict__)
        name = f.getName()
        self.assertEqual(name, 'bibo')


    def test_toPath(self):
        f = self.File('/usr/local/bibo')
        self.assertTrue('toPath' in self.File.__dict__)
        path = f.toPath()
        self.assertEqual(str(type(path)), '<class \'java.nio.file.Path\'>')

        jpy.get_class('java.nio.file.Path')
        n1 = path.getName(0)
        n2 = path.getName(1)
        n3 = path.getName(2)
        self.assertEqual(str(n1), 'usr')
        self.assertEqual(str(n2), 'local')
        self.assertEqual(str(n3), 'bibo')


class TestArrayList(unittest.TestCase):

    def setUp(self):
        self.ArrayList = jpy.get_class('java.util.ArrayList')

        array_list = ArrayList()
        array_list.add('A')
        array_list.add(12)
        array_list.add(3.4)

        self.assertEqual(array_list.size(), 3)
        self.assertEqual(array_list.get(0), 'A')
        self.assertEqual(array_list.get(1), 12)
        self.assertEqual(array_list.get(2), 3.4)

        array_list = ArrayList(array_list)
        self.assertEqual(array_list.size(), 3)
        self.assertEqual(array_list.get(0), 'A')
        self.assertEqual(array_list.get(1), 12)
        self.assertEqual(array_list.get(2), 3.4)

        array = array_list.toArray()
        self.assertEqual(len(array), 3)
        self.assertEqual(array[0], 'A')
        self.assertEqual(array[1], 12)
        self.assertEqual(array[2], 3.4)


class TestHashMap(unittest.TestCase):

    def setUp(self):
        self.HashMap = jpy.get_class('java.util.HashMap')

        hash_map = HashMap()
        hash_map.put(0, 'A')
        hash_map.put(1, 12)
        hash_map.put(2, 3.4)

        self.assertEqual(hash_map.size(), 3)
        self.assertEqual(hash_map.get(0), 'A')
        self.assertEqual(hash_map.get(1), 12)
        self.assertEqual(hash_map.get(2), 3.4)

        hash_map = HashMap(hash_map)
        self.assertEqual(hash_map.size(), 3)
        self.assertEqual(hash_map.get(0), 'A')
        self.assertEqual(hash_map.get(1), 12)
        self.assertEqual(hash_map.get(2), 3.4)



if __name__ == '__main__':
    print('\nRunning', __file__)
    unittest.main()
