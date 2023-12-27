import unittest
from timer import Timer, Stopwatch
from datetime import timedelta
from time import sleep

class TestAnt(unittest.TestCase):

  def setUp(self):
    pass

  def test_timer(self):
    t = Timer(timedelta(seconds=3))
    self.assertFalse(t.expired())
    t.expire()
    self.assertTrue(t.expired())
    sleep(3.5)
    self.assertTrue(t.expired())
    

  def test_stopwatch(self):
    s = Stopwatch()
    self.assertGreater(s.read(), 0)
    self.assertLess(s.read(), 1)
    sleep(1)
    self.assertGreater(s.read(), 1)
    s.stop()
    elapsed = s.read()
    sleep(0.25)
    self.assertEqual(s.read(), elapsed)
    s.start()
    self.assertLess(s.read(), 1)
    
    

if __name__ == '__main__':
    unittest.main()
