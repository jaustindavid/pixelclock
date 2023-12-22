#!/usr/bin/env python

from typing import List
from math import sqrt
import random
from defs import constrain


class Pixel:
  def __init__(self, x=0, y=0, color: str = ' '):
    self.x = constrain(x)
    self.y = constrain(y)
    self.color = color


  def __str__(self):
    return f"({self.x},{self.y}):{self.color}"


  def __eq__(self, other: any):
    return self.x == other.x and self.y == other.y


  # def __hash__(self):
  #   return hash((self.x, self.y, self.color))


  # returns a clone of myself
  def clone(self):
    return Pixel(self.x, self.y, self.color)


  """ Translate self to another pixel """
  def translate(self, dx=0, dy=0, pixel=None):
    if pixel:
      self.x = constrain(self.x + pixel.x)
      self.y = constrain(self.y + pixel.y)
    else:
      self.x = constrain(self.x + dx)
      self.y = constrain(self.y + dy)


  def distance_to(self, other):
    return sqrt((other.x-self.x)**2 + (other.y-self.y)**2)
  

  # returns others, sorted by distance
  def nearish(self, others: List[any]) -> List[any]:
    s = {}
    for other in others:
      s[self.distance_to(other)] = other
    return [value for key, value in sorted(s.items())] 


  def step(self, dx: int, dy: int, sandbox: List[any]) -> bool:
    p = self.clone()
    p.translate(dx, dy)
    if p not in sandbox:
      self.translate(dx, dy)
      return True
    return False


  # returns a step in the right direction
  # wobble: probability that I might wobble from a straight line
  def _d(self, x, target: int, wobble: float = 0.0):
    if target > x:
      weights = [ 0, wobble, 1-wobble ]
    elif target < x:
      weights = [ 1-wobble, wobble, 0 ]
    else:
      weights = [ wobble/2, 1-wobble, wobble/2 ]
    return constrain(random.choices([-1, 0, 1], weights=weights)[0])


  def seek(self, targets: List[any], sandbox: List[any], wobble: float = 0.0):
    target = targets[0]
    dx = self._d(self.x, target.x, wobble)
    dy = self._d(self.y, target.y, wobble)
    if dx or dy:
      return self.step(dx, dy, sandbox)
    else:
      return True


class Food(Pixel):
  def __str__(self):
    return f"Food: {super().__str__()}"


  # "==" same as "collides with"
  def __eq__(self, other):
    if super().__eq__(other):
      if isinstance(other, Food):
        return True
    return False


  def __hash__(self):
    return hash(("food", self.x, self.y, self.color))


  def clone(self):
    return Food(self.x, self.y, self.color)


class Ant(Pixel):
  def __str__(self):
    return f"Ant: {super().__str__()}"


  # "==" same as "collides with"
  def __eq__(self, other):
    if super().__eq__(other):
      if not isinstance(other, Food):
        return True
    return False


  def __hash__(self):
    return hash(("ant", self.x, self.y, self.color))


  def clone(self):
    return Ant(self.x, self.y, self.color)


