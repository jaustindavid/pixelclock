import unittest
from datetime import datetime
import time
import defs
from ping import ping_forever
from mproc import ForeverProcess
import internet_quality

class TestIQ(unittest.TestCase):

  def setUp(self):
    pass

  def tearDown(self):
    pass

  def test_run(self):
    iq = internet_quality.InternetQuality([], 15)
    start = datetime.now()
    while iq.age > 60:
      self.assertLess((datetime.now() - start).total_seconds(), 120)
      iq.run()
      time.sleep(1)
    self.assertLess(iq.age, 60)

  def test_graph(self):
    print("This test takes 2 minutes, be patient")
    iq = internet_quality.InternetQuality([], 15)
    start = datetime.now()
    while (datetime.now() - start).total_seconds() < 120:
      iq.run()
      time.sleep(5)
      print(defs.listr(iq.graph))


if __name__ == '__main__':
    unittest.main()
