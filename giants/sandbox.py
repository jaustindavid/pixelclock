from pixel import Pixel
from matrix import Matrix

class Sandbox:
  def __init__(self):
    self.stuff = {}


  def __str__(self):
    ret = "["
    i = 0
    for item in self.stuff:
      ret += f"{i}: {item}, "
      i += 1
    ret += "]"
    return ret


  def __contains__(self, item):
    return item in self.stuff


  def add(self, item: Pixel) -> bool:
    if item not in self.stuff:
      self.stuff[item] = True
      return True
    return False


  def remove(self, item: Pixel):
    if item in self.stuff:
      del self.stuff[item]
