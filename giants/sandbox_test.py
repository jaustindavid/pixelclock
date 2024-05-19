import unittest
from pixel import Pixel, Ant, Food
from sandbox import Sandbox

class TestSandbox(unittest.TestCase):

  def setUp(self):
    self.sandbox = Sandbox()

  def test_init(self):
    self.assertTrue(isinstance(self.sandbox, Sandbox))

  def test_add_remove(self):
    p = Pixel()
    self.assertTrue(self.sandbox.add(p))
    self.assertFalse(self.sandbox.add(p))
    self.assertIn(p, self.sandbox)
    self.sandbox.remove(p)
    self.assertNotIn(p, self.sandbox)
    ant = Ant(0,0)
    food1 = Food(0,1)
    ant1 = Ant(0,1)
    food = Food(0,1)
    self.assertTrue(self.sandbox.add(food))
    self.assertTrue(self.sandbox.add(ant))
    self.assertTrue(self.sandbox.add(ant1))
    self.assertFalse(self.sandbox.add(food1))
    self.assertTrue(food1.step(0, -1))
    self.assertTrue(self.sandbox.add(food1))
    
  def test_step(self):
    p = Pixel(0,0)
    self.assertTrue(self.sandbox.add(p))
    p2 = Pixel(0,1)
    self.assertTrue(self.sandbox.add(p))
    p2p = p2.clone()
    self.assertTrue(p2 in self.sandbox)


if __name__ == '__main__':
    unittest.main()
