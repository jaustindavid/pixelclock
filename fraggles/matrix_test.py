import unittest
from pixel import Pixel, Ant, Food
from matrix import Matrix

class TestMatrix(unittest.TestCase):

  def setUp(self):
    self.matrix = Matrix(16, [])

  def test_print(self):
    self.matrix.sandbox.extend([Ant(i,i,'w') for i in range(self.matrix.size)])
    self.assertEqual(str(self.matrix).count('w'), self.matrix.size)
    

if __name__ == '__main__':
    unittest.main()
