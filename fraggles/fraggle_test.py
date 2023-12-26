import unittest
import time
import defs
from pixel import Pixel
import fraggle as f
from fraggle import Fraggle
from matrix import Matrix

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
    while i < 10 and fraggle == Pixel(2,2):
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


  def test_fetch2(self):
    print("FETCH2")
    fraggle = Fraggle()
    fraggle2 = Fraggle()

    # generate a lite plan:
    plans = [ Pixel(2,2,' ') ]
    sandbox = [ fraggle, fraggle2, Pixel(4,4,f.BRICK_COLOR) ]
    print(f"FETCH 2 start sandbox: {defs.listr(sandbox)}")

    # generate competition 
    fraggle.state = f.FETCHING
    fraggle.run(plans, sandbox, debug=True)
    print(f"F1:{fraggle} => {fraggle.sought_target}")
    print(f"sandbox: {defs.listr(sandbox)}")
    fraggle2.state = f.FETCHING
    fraggle2.run(plans, sandbox, debug=True)
    print(f"F2:{fraggle2} => {fraggle2.sought_target}")
    print(f"sandbox: {defs.listr(sandbox)}")

    # let #1 win
    while fraggle.state == f.FETCHING:
      print(f"F1:{fraggle} => {fraggle.sought_target}, plans={defs.listr(plans)}")
      fraggle.fetch(plans, sandbox, debug=True)
    print(f"F1: {fraggle}, sandbox: {defs.listr(sandbox)}")
    self.assertNotIn(Pixel(4,4,f.BRICK_COLOR), sandbox)

    # move #2 to the finish line
    fraggle2.x = 4
    fraggle2.y = 4
    print(f"F2:{fraggle2}: {defs.listr(sandbox)}")

    # have f2 attempt to fetch
    print(f"F2:{fraggle2} => {fraggle2.sought_target}, {defs.listr(sandbox)}")
    fraggle2.fetch(plans, sandbox, debug=True)
    print(f"F2:{fraggle2} => {fraggle2.sought_target}, {defs.listr(sandbox)}")

    self.assertNotEqual(fraggle2.state, f.FETCHING)


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
    print("CLEAN")
    fraggle = Fraggle()
    i = 0
    print(f"plans: {defs.listr(self.plans)}")
    print(f"sandbox: {defs.listr(self.sandbox)}")
    while i < 3 and fraggle == Pixel():
      fraggle.clean(self.plans, self.sandbox, debug=True)
      print(f"{fraggle}")
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
    print("DUMPING")
    fraggle = Fraggle()
    i = 0
    # in dump mode, fraggle should start moving toward (15,14)
    while i < 3 and fraggle == Pixel():
      fraggle.dump(self.plans, self.sandbox, debug=True)
      i += 1
    self.assertNotEqual(fraggle, Pixel())

    fraggle.x = 15
    fraggle.y = 14

    print(f"{fraggle}: {defs.listr(self.sandbox)}")
    self.assertNotIn(Pixel(15,14), self.sandbox)
    fraggle.dump(self.plans, self.sandbox, debug=True)
    fraggle.step(-1, 0, self.sandbox)
    self.assertEqual(fraggle.state, f.RESTING)


  def test_cleaning_cycle(self):
    print("E2E CLEANING")
    fraggle = Fraggle()
    plans = [ Pixel(1,1,' '), 
              Pixel(1,2,' '),
              Pixel(1,3,' '),
              Pixel(1,4,' ') ]
    sandbox = [ fraggle, 
                Pixel(0,14,f.BRICK_COLOR), 
                Pixel(0,13,f.BRICK_COLOR), 
                Pixel(0,12,f.BRICK_COLOR), 
                Pixel(1,1,f.BRICK_COLOR),
                Pixel(1,2,f.BRICK_COLOR),
                Pixel(1,3,f.BRICK_COLOR),
                Pixel(1,4,f.BRICK_COLOR) ]
    matrix = Matrix(defs.SIDE, sandbox)
    print(matrix)
    fraggle.run(plans, sandbox)
    self.assertEqual(fraggle.state, f.CLEANING)
    i = 0
    while i < 100:
      fraggle.run(plans, sandbox, debug=True)
      print(" -------------- ")
      print(matrix)
      time.sleep(0.25)
      i += 1
    self.assertNotIn(Pixel(0,14), sandbox)


  def test_stuck(self):
    fraggle = Fraggle()
    fraggle.stuckness = 1 + f.STUCK_COUNTER
    fraggle.am_i_stuck()
    self.assertEqual(fraggle.state, f.STUCK)
    fraggle.run(self.plans, self.sandbox)
    self.assertEqual(fraggle.state, f.STUCK)
    i = 0
    # should wander and eventually unstick
    while i < 20 and fraggle.state == f.STUCK:
      fraggle.run(self.plans, self.sandbox)
      i += 1
    print(f"{fraggle}")
    self.assertNotEqual(fraggle.state, f.STUCK)



if __name__ == '__main__':
    unittest.main()
