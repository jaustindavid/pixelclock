import unittest
from ant import Ant
from pixel import Pixel
from matrix import Matrix

class TestAnt(unittest.TestCase):

  def setUp(self):
    self.matrix = Matrix()
    pass

  def test_init(self):
    a = Ant(self.matrix, 'W')
    self.assertEqual(str(a), "W@(0,0)")

  def test_randomize(self):
    a = Ant(self.matrix, 'W')
    a.randomize()
    self.assertEqual(self.matrix.get(a), 'W')
    self.assertEqual(self.matrix.get(Pixel(a.x, a.y)), 'W')

  def test_step(self):
    a = Ant(self.matrix, 'W')
    a.step(1,1)
    self.assertEqual(a.pixel, Pixel(1,1))

  def test_walkTo(self):
    a = Ant(self.matrix, 'W')
    targetPixel = Pixel(2,2)
    self.assertTrue(a.walkTo(targetPixel=targetPixel))

if __name__ == '__main__':
    unittest.main()
