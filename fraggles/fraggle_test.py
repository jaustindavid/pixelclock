import unittest
import defs
from pixel import Pixel
import fraggle as f
from fraggle import Fraggle

class TestFraggle(unittest.TestCase):

  def setUp(self):
    self.plans = [ Pixel(1,1,' '), 
                   Pixel(1,2,' '),
                   Pixel(1,3,' '),
                   Pixel(1,4,' ') ]
    self.sandbox = [ Pixel(1,2,f.BRICK_COLOR),
                     Pixel(5,5,f.BRICK_COLOR),
                     Pixel(2,5,'q') ]


  def test_init(self):
    fraggle = Fraggle()
    self.assertEqual(fraggle, Pixel())
    self.assertEqual(fraggle.state, f.RESTING)


  def test_find(self):
    fraggle = Fraggle()
    self.assertIn(Pixel(1,2,f.BRICK_COLOR),
                  fraggle.find(f.BRICK_COLOR, self.sandbox))
    self.assertNotIn(Pixel(2,5,'q'),
                     fraggle.find(f.BRICK_COLOR, self.sandbox))
    self.assertFalse(fraggle.find(' ', self.sandbox))


  def test_rest(self):
    # fetching
    fraggle = Fraggle()
    fraggle.rest(self.plans, self.sandbox)
    self.assertEqual(fraggle, Pixel())
    self.assertEqual(fraggle.state, f.FETCHING)

    # generate a dirty plan:
    plans = [ Pixel(1,1,' '), 
              Pixel(1,2,' '),
              Pixel(1,3,' '),
              Pixel(1,4,' ') ]
    sandbox = [ Pixel(1,1,f.BRICK_COLOR), 
                Pixel(1,2,f.BRICK_COLOR),
                Pixel(1,3,f.BRICK_COLOR),
                Pixel(1,4,f.BRICK_COLOR),
                Pixel(1,5,f.BRICK_COLOR) ]

    # cleaning
    fraggle = Fraggle()
    fraggle.rest(plans, sandbox)
    self.assertEqual(fraggle, Pixel())
    self.assertEqual(fraggle.state, f.CLEANING)

    # generate a full plan:
    plans = [ Pixel(1,1,' '), 
              Pixel(1,2,' '),
              Pixel(1,3,' '),
              Pixel(1,4,' ') ]
    sandbox = [ Pixel(1,1,f.BRICK_COLOR), 
                Pixel(1,2,f.BRICK_COLOR),
                Pixel(1,3,f.BRICK_COLOR),
                Pixel(1,4,f.BRICK_COLOR) ]

    # near nothing, do nothing
    fraggle = Fraggle()
    fraggle.translate(5,5)
    self.assertFalse(fraggle.adjacent(sandbox))
    fraggle.rest(plans, sandbox)
    print(fraggle)
    self.assertEqual(fraggle, Pixel(5,5))

    # near something, wander
    fraggle = Fraggle()
    fraggle.translate(2,2)
    self.assertEqual(fraggle, Pixel(2,2))
    i = 0
    # this can be flaky, ~ 1/6 chance of doing nothing
    while i < 3 and fraggle == Pixel(2,2):
      fraggle.rest(plans, sandbox)
      i += 1
    self.assertNotEqual(fraggle, Pixel(2,2))


  def test_fetch(self):
    # fetching
    fraggle = Fraggle()
    i = 0
    while i < 3 and fraggle == Pixel():
      fraggle.fetch(self.plans, self.sandbox)
      i += 1
    self.assertNotEqual(fraggle, Pixel())

    # picking up
    fraggle = Fraggle()
    fraggle.translate(5, 4)
    self.assertEqual(fraggle, Pixel(5,4))
    self.assertIn(Pixel(5,5), self.sandbox)
    fraggle.fetch(self.plans, self.sandbox)
    self.assertNotIn(Pixel(5,5), self.sandbox)
    self.assertEqual(fraggle.state, f.BUILDING)


  def test_build(self):
    fraggle = Fraggle()
    fraggle.translate(5, 4)
    self.assertEqual(fraggle, Pixel(5,4))
    self.assertIn(Pixel(5,5), self.sandbox)
    fraggle.fetch(self.plans, self.sandbox)
    self.assertNotIn(Pixel(5,5), self.sandbox)
    self.assertEqual(fraggle.state, f.BUILDING)
    self.assertNotIn(Pixel(1,4), self.sandbox)
    fraggle.x = 1
    fraggle.y = 4
    fraggle.build(self.plans, self.sandbox)
    fraggle.step(1, 0, self.sandbox)
    self.assertIn(Pixel(1,4), self.sandbox)
    self.assertEqual(fraggle.state, f.RESTING)


  def test_clean(self):
    fraggle = Fraggle()
    i = 0
    while i < 3 and fraggle == Pixel():
      fraggle.clean(self.plans, self.sandbox)
      i += 1
    self.assertNotEqual(fraggle, Pixel())

    fraggle = Fraggle()
    fraggle.x = 4
    fraggle.y = 4
    self.assertIn(Pixel(5,5), self.sandbox)
    fraggle.clean(self.plans, self.sandbox)
    self.assertNotIn(Pixel(5,5), self.sandbox)
    self.assertEqual(fraggle.state, f.DUMPING)


  def test_dump(self):
    fraggle = Fraggle()
    i = 0
    while i < 3 and fraggle == Pixel():
      fraggle.dump(self.plans, self.sandbox)
      i += 1
    self.assertNotEqual(fraggle, Pixel())

    fraggle.x = 15
    fraggle.y = 15

    self.assertNotIn(Pixel(15,15), self.sandbox)
    fraggle.dump(self.plans, self.sandbox)
    fraggle.step(-1, 0, self.sandbox)
    self.assertIn(Pixel(15,15), self.sandbox)
    self.assertEqual(fraggle.state, f.RESTING)


if __name__ == '__main__':
    unittest.main()
