import unittest
from pixel import Pixel, _choose_weighted_random

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
    self.assertEqual(a.distance_to(b), 0)
    self.assertEqual(a.distance_to(c), 5)


  def test_step(self):
    dot = Pixel(1,1)
    sandbox = []
    sandbox.append(dot)
    self.assertIn(dot, sandbox)
    pot = dot.clone()
    self.assertIn(pot, sandbox)
    pot.x += 1
    self.assertNotIn(pot, sandbox)
    pot.step(0, 0, sandbox)
    dot = Pixel(0,1)
    self.assertNotIn(dot, sandbox)
    self.assertTrue(dot.step(0,-1, sandbox))
    self.assertFalse(dot.step(1,1, sandbox))


  def test_scan(self):
    dot = Pixel(1,1)
    sandbox = []
    sandbox.append(dot)
    self.assertIn(dot, sandbox)
    p = Pixel(0,0)
    self.assertNotIn(p, sandbox)
    p.x = 1
    p.y = 1
    self.assertIn(p, sandbox)
    

  def test_seek(self):
    ant = Pixel(0,0, 'w')
    food = [ Pixel(1,1,'g') ]
    sandbox = [ ant ]
    i = 0
    while i < 10 and ant.distance_to(food[0]) > 0:
      self.assertTrue(ant.seek(food, sandbox))
      i += 1
    self.assertTrue(ant.distance_to(food[0]) == 0)

  def test_nearest(self):
    ant = Pixel(0,0, 'w')
    food = [ Pixel(1,1,'g'), Pixel(3,3,'g') ]
    print(f"nearest: {[str(other) for other in ant.nearest(food)]}")
    self.assertEqual(Pixel(1,1,'g'), ant.nearest(food)[0])
    ant = Pixel(10,10, 'w')
    print(f"nearest: {[str(other) for other in ant.nearest(food)]}")
    self.assertEqual(Pixel(3,3,'g'), ant.nearest(food)[0])

  def test_weighted_rando(self):
    data = [4, 2, 5, 1, 3]
    r = {}
    for i in range(1000):
      d = _choose_weighted_random(data)
      if d in r:
        r[d] += 1
      else:
        r[d] = 1
    print(r)
    self.assertTrue(r[1] >= r[3] >= r[5])


  def test_nearish(self):
    ant = Pixel(0,0, 'w')
    food = [ Pixel(1,1,'g'), Pixel(3,3,'g') ]
    nearest = 0
    for i in range(1000):
      if food[0] == ant.nearish(food):
        nearest += 1 
    self.assertGreater(nearest, 500)
    ant = Pixel(10,10, 'w')
    nearest = 0
    for i in range(1000):
      if food[0] == ant.nearish(food):
        nearest += 1 
    self.assertLess(nearest, 500)


  def test_open(self):
    food = [ Pixel(1,1,'g'), Pixel(3,3,'g') ]
    ants = [ Pixel(0,1,'w'), Pixel(3,3,'w') ]
    ant = Pixel()
    self.assertIsNot(ant.open(food, ants), None)
    self.assertIn(food[0], ant.open(food, ants))


  def test_adjacent(self):
    food = [ Pixel(1,1,'g'), Pixel(3,3,'g') ]
    ant = Pixel()
    self.assertIsNot(ant.adjacent(food), None)
    self.assertIs(food[0], ant.adjacent(food))

if __name__ == '__main__':
    unittest.main()
