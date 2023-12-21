import random

from pixel import Pixel
from matrix import Matrix


class Ant(Pixel):
  def __init__(self, matrix, color, pixel=Pixel(0,0)):
    self.matrix = matrix
    self.color = color
    super().__init__(x=pixel.x, y=pixel.y)
    self.matrix.set(self)


  def __del__(self):
    if self.matrix.get(self.pixel) == self.color:
      self.matrix.unset(self.pixel)


  def immolate(self): 
    self.matrix.set(self.pixel, 'R')
    self.matrix.unset(self.pixel)


  def randomize(self):
    while True:
      p = Pixel(random.randint(0,15), random.randint(0,15))
      if not self.matrix.get(p) or self.matrix.get(p) == 'X':
        break
    self.pixel = p
    self.matrix.set(self.pixel, self.color)


  def __str__(self):
    return f"{self.color}@{self.pixel}"


  def step(self, dx: int, dy: int):
    newPixel = Pixel(pixel=self.pixel)
    newPixel.translate(dx, dy)
    if not self.matrix.get(newPixel) \
        or self.matrix.get(newPixel) == '.':
      self.matrix.unset(self.pixel)
      self.pixel = newPixel
      self.matrix.set(self.pixel, self.color)
      return True
    return False


  def jumpTo(self, x: int = 0, y: int = 0, newPixel: Pixel = None):
    self.matrix.unset(self.pixel)
    self.pixel = newPixel
    self.matrix.set(self.pixel, self.color)


  def walkTo(self, targetX=0, targetY=0, 
                   targetPixel: Pixel = None,
                   wobble=0.0):
    def _direct(x, xPrime, wobble=0.0):
      weights = [ 0.025, 0.95, 0.025 ]
      weights = [ wobble/2, 1-wobble, wobble/2 ]
      if xPrime > x:
        weights = [ wobble, 0.2*(1-wobble), 0.8*(1-wobble) ]
      if xPrime < x:
        weights = [ 0.8*(1-wobble), 0.2*(1-wobble), wobble ]
      c = random.choices([-1, 0, 1], weights=weights)[0]
      return c

    if self.color == 'D':
      print(f"{self} walkTo {targetPixel}, p={wobble}")
    if targetPixel:
      targetX = targetPixel.x
      targetY = targetPixel.y
    
    ntries = 0
    while ntries < 3:
      dx = _direct(self.pixel.x, targetX, wobble)
      dy = _direct(self.pixel.y, targetY, wobble)
      if self.color == 'D':
        print(f"{self} ?= ({dx},{dy})")
      if self.step(dx, dy):
        if self.color == 'D':
          print(f"{self} += ({dx},{dy})")
        return True
      ntries += 1
    return False


  def wander(self) -> bool:
    ntries = 0
    while ntries < 8:
      dx = random.choice([-1, 0, 1])
      dy = random.choice([-1, 0, 1])
      if self.step(dx, dy):
        return True
      ntries += 1
    return False
