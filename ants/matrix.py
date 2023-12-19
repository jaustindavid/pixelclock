#!/usr/bin/env python

from time import sleep
import random
from point import Point

import board
import neopixel

from colors import COLOR

SIDE = 16
class Matrix:
  def __init__(self):
    self.matrix = [ [ ' ' for _ in range(SIDE) ] for _ in range(SIDE) ]
    self.pixels = neopixel.NeoPixel(board.D18, 256)
    self.pixels.fill(COLOR[' '])

  def show(self):
    for row in self.matrix:
      print(" ".join(row))

  def set(self, point, color):
    # translates Point to a pixel location
    def _pixel() -> int:
      # print(f"Trying {point}: x={point.x}, y={point.y}")
      if point.x % 2 == 0:
        p = point.x * SIDE + point.y
      else:
        p = point.x * SIDE + SIDE - point.y - 1
      # print(f"returning {p}")
      return p

    self.matrix[point.y][point.x] = color
    if color in COLOR:
      rgb = COLOR[color]
    else:
      print(f"WARNING: color '{color}' not defined")
      rgb = (0,0,0)
    self.pixels[_pixel()] = rgb

  def unset(self, point):
    self.set(point, ' ')

  def get(self, point):
    if self.matrix[point.y][point.x] != ' ' \
       and self.matrix[point.y][point.x] != '.':
      return self.matrix[point.y][point.x]
    return None

  # returns a value hard-constrained to the square boundary
  def constrain(value):
    if value < 0:
      return 0
    if value > SIDE - 1:
      return SIDE -1 
    return value
