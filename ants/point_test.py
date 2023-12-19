import unittest
from point import Point

class TestPoint(unittest.TestCase):

  def setUp(self):
    pass

  def test_init(self):
    p = Point(1,1)
    self.assertEqual(p.x, 1)
    self.assertEqual(p.y, 1)
    p2 = Point(point=p)
    self.assertEqual(p2.x, 1)
    self.assertEqual(p2.y, 1)

  def test_str(self):
    p = Point(1,1)
    self.assertEqual(str(p), "(1,1)")

  def test_eq(self):
    p = Point(1,1)
    p2 = Point(1,1)
    c = Point(1,2)
    self.assertEqual(p, p2)
    self.assertNotEqual(p, c)


  def test_translate(self):
    p = Point(1,1)
    p.translate(point=Point(3,4))
    self.assertEqual(p.x, 4)
    self.assertEqual(p.y, 5)
    
  def test_distance(self):
    a = Point(1,1)
    b = Point(1,1)
    c = Point(4,5)
    self.assertEqual(a.distanceTo(b), 0)
    self.assertEqual(a.distanceTo(c), 5)
   
    

if __name__ == '__main__':
    unittest.main()
