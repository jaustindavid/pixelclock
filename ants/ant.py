import random

from point import Point
from matrix import Matrix


class Ant:
  def __init__(self, matrix, color, point=Point(0,0)):
    self.matrix = matrix
    self.color = color
    if point:
      self.point = Point(point=point)
    self.matrix.set(self.point, self.color)


  def __del__(self):
    if self.matrix.get(self.point) == self.color:
      self.matrix.unset(self.point)


  def immolate(self): 
    self.matrix.set(self.point, 'R')
    self.matrix.unset(self.point)


  def randomize(self):
    while True:
      p = Point(random.randint(0,15), random.randint(0,15))
      if not self.matrix.get(p) or self.matrix.get(p) == 'X':
        break
    self.point = p
    self.matrix.set(self.point, self.color)


  def __str__(self):
    return f"{self.color}@{self.point}"


  def step(self, dx: int, dy: int):
    newPoint = Point(point=self.point)
    newPoint.translate(dx, dy)
    if not self.matrix.get(newPoint) \
        or self.matrix.get(newPoint) == '.':
      self.matrix.unset(self.point)
      self.point = newPoint
      self.matrix.set(self.point, self.color)
      return True
    return False


  def jumpTo(self, x: int = 0, y: int = 0, newPoint: Point = None):
    self.matrix.unset(self.point)
    self.point = newPoint
    self.matrix.set(self.point, self.color)


  def walkTo(self, targetX=0, targetY=0, 
                   targetPoint: Point = None,
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
      print(f"{self} walkTo {targetPoint}, p={wobble}")
    if targetPoint:
      targetX = targetPoint.x
      targetY = targetPoint.y
    
    ntries = 0
    while ntries < 3:
      dx = _direct(self.point.x, targetX, wobble)
      dy = _direct(self.point.y, targetY, wobble)
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
