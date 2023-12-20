import unittest
from point import Point
from font import decode, pstr, bar_graph

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


  def test_bar_graph(self):
    graph = bar_graph(origin=Point(0,15), height=3)
    self.assertEqual(len(graph), 3)
    self.assertIn(Point(0,15), graph)
    self.assertIn(Point(0,13), graph)
    

if __name__ == '__main__':
    unittest.main()
