import unittest
from point import Point
from font import decode, pstr

class TestAnt(unittest.TestCase):

  def setUp(self):
    pass

  def test_decode(self):
    p = decode('X X X')
    self.assertEqual(p[0], Point(0,0))
    p = decode('X X X\nX X X')
    self.assertEqual(p[3], Point(0,1))

  def test_pstr(self):
    p = decode('X X X')
    self.assertEqual(pstr(p), "[(0,0),(2,0),(4,0)]")

if __name__ == '__main__':
    unittest.main()
