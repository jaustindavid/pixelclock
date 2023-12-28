import unittest
import defs
from pixel import Ant

class TestPixel(unittest.TestCase):

  def setUp(self):
    pass


  def test_constrain(self):
    self.assertEqual(defs.constrain(11), 11)
    self.assertEqual(defs.constrain(-11), 0)
    self.assertEqual(defs.constrain(99), defs.SIDE-1)


  def test_map_basic(self):
    self.assertEqual(defs.map_basic(11, 1, 10, 1, 10), 10)
    self.assertEqual(defs.map_basic(50, 0, 100, 0, 10), 5)


  def test_listr(self):
    l = [ Ant(), Ant() ]
    self.assertEqual(defs.listr(l), f"[{Ant()}, {Ant()}]")


if __name__ == '__main__':
    unittest.main()
