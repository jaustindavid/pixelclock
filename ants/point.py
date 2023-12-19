#!/usr/bin/env python

from math import sqrt
from defs import constrain

class Point:
  def __init__(self, x=0, y=0, point=None):
    if point:
      self.x = point.x
      self.y = point.y
    else:
      self.x = constrain(x)
      self.y = constrain(y)


  def __str__(self):
    return f"({self.x},{self.y})"


  def __iter__(self):
    yield self.x
    yield self.y


  def __next__(self):
    raise StopIteration()


  def __eq__(self, other):
    return self.x == other.x and self.y == other.y


  """ Translate self to another point """
  def translate(self, dx=0, dy=0, point=None):
    if point:
      self.x = constrain(self.x + point.x)
      self.y = constrain(self.y + point.y)
    else:
      self.x = constrain(self.x + dx)
      self.y = constrain(self.y + dy)


  def distanceTo(self, other):
    return sqrt((other.x-self.x)**2 + (other.y-self.y)**2)
