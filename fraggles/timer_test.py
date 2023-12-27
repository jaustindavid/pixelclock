import unittest
from timer import Timer
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
    

if __name__ == '__main__':
    unittest.main()
