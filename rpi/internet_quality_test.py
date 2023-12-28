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

  def test_scores(self):
    iq = internet_quality.InternetQuality(15)
    iq.age = 10
    self.assertEqual(iq.age_score(), 4)
    iq.rtt = 15
    self.assertEqual(iq.rtt_score(), 4)



  def test_run(self):
    iq = internet_quality.InternetQuality(15)
    start = datetime.now()
    while iq.age > 60:
      self.assertLess((datetime.now() - start).total_seconds(), 120)
      iq.run()
      time.sleep(1)
    self.assertLess(iq.age, 60)

if __name__ == '__main__':
    unittest.main()
