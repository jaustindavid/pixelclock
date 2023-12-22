import unittest
from pixel import Pixel
import font

class TestAnt(unittest.TestCase):

  def setUp(self):
    pass

  def test_decode(self):
    p = font.decode('X X X')
    self.assertEqual(p[0], Pixel(0,0))
    p = font.decode('X X X\nX X X')
    self.assertEqual(p[3], Pixel(0,1))

  def test_get_time(self):
    pixels = font.get_time()
    self.assertGreater(len(pixels), 10)

if __name__ == '__main__':
    unittest.main()
