import unittest
from pixel import Pixel, Food, Ant

class TestPixel(unittest.TestCase):

  def setUp(self):
    pass

  def test_init(self):
    p = Pixel(1,1)
    self.assertEqual(p.x, 1)
    self.assertEqual(p.y, 1)
    self.assertEqual(p.color, ' ')


  def test_str(self):
    p = Pixel(1,1)
    self.assertEqual(str(p), "(1,1): ")


  def test_eq(self):
    p = Pixel(1,1)
    p2 = Pixel(1,1)
    c = Pixel(1,2)
    self.assertEqual(p, p2)
    self.assertNotEqual(p, c)


  def test_clone(self):
    p = Pixel(1,1)
    p2 = p.clone()
    self.assertIsNot(p, p2)
    self.assertEqual(p, p2)


  def test_translate(self):
    p = Pixel(1,1)
    p.translate(pixel=Pixel(3,4))
    self.assertEqual(p.x, 4)
    self.assertEqual(p.y, 5)
    

  def test_distance(self):
    a = Pixel(1,1)
    b = Pixel(1,1)
    c = Pixel(4,5)
    self.assertEqual(a.distanceTo(b), 0)
    self.assertEqual(a.distanceTo(c), 5)


  def test_collision(self):
    ant = Ant(1,1)
    food = Food(1,1)
    self.assertNotEqual(ant, food)
    food2 = food.clone()
    self.assertNotEqual(ant, food2)
    self.assertEqual(food, food2)
    sandbox = []
    ant = Ant(1,1)
    sandbox.append(ant)
    self.assertIn(ant, sandbox)
    food = Food(1,1)
    sandbox.append(food)
    self.assertIn(food, sandbox)
    food2 = food.clone()
    self.assertIn(food2, sandbox)
    pixel = Pixel(1,1)
    self.assertIn(pixel, sandbox)


  def test_step(self):
    ant = Ant(1,1)
    sandbox = []
    sandbox.append(ant)
    self.assertIn(ant, sandbox)
    pant = ant.clone()
    self.assertIn(pant, sandbox)
    pant.x += 1
    self.assertNotIn(pant, sandbox)
    ant.step(0, 0, sandbox)
    ant = Ant(0,1)
    self.assertNotIn(ant, sandbox)
    self.assertTrue(ant.step(0,-1, sandbox))
    self.assertFalse(ant.step(1,1, sandbox))
    food = Food(0,0)
    sandbox.append(food)
    self.assertIn(food, sandbox)
    self.assertTrue(ant.step(-1,-1, sandbox))

  def test_scan(self):
    ant = Ant(1,1)
    sandbox = []
    sandbox.append(ant)
    self.assertIn(ant, sandbox)
    p = Pixel(0,0)
    self.assertNotIn(p, sandbox)
    p.x = 1
    p.y = 1
    self.assertIn(p, sandbox)
    
  def test_seek(self):
    ant = Ant(0,0, 'w')
    food = [ Food(1,1,'g') ]
    sandbox = [ ant ]
    self.assertTrue(ant.seek(food, sandbox))


if __name__ == '__main__':
    unittest.main()
