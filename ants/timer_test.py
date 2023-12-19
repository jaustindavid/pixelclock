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
    sleep(3)
    self.assertTrue(t.expired())
    

if __name__ == '__main__':
    unittest.main()
