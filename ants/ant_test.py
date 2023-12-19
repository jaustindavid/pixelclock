import unittest
from ant import Ant
from point import Point
from matrix import Matrix

class TestAnt(unittest.TestCase):

  def setUp(self):
    self.matrix = Matrix()
    pass

  def test_init(self):
    a = Ant(self.matrix, 'A')
    self.assertEqual(str(a), "A@(0,0)")

  def test_randomize(self):
    a = Ant(self.matrix, 'A')
    a.randomize()
    self.assertEqual(self.matrix.get(a.point), 'A')

  def test_step(self):
    a = Ant(self.matrix, 'A')
    a.step(1,1)
    self.assertEqual(a.point, Point(1,1))

  def test_walkTo(self):
    a = Ant(self.matrix, 'A')
    targetPoint = Point(2,2)
    self.assertTrue(a.walkTo(targetPoint=targetPoint))

if __name__ == '__main__':
    unittest.main()
