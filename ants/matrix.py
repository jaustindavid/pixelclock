#!/usr/bin/env python

from time import sleep
import random
from point import Point

CLI_MODE = False
try:
  import board
  import neopixel
except ModuleNotFoundError as e:
  print("Failed to import board OR neopixel; running CLI-only")
  CLI_MODE = True

  

from colors import COLOR

SIDE = 16
class Matrix:
  def __init__(self):
    self.matrix = [ [ ' ' for _ in range(SIDE) ] for _ in range(SIDE) ]
    if not CLI_MODE:
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
    if not CLI_MODE:
      self.pixels[_pixel()] = rgb


  def unset(self, point):
    self.set(point, ' ')


  def get(self, point):
    if self.matrix[point.y][point.x] != ' ' \
       and self.matrix[point.y][point.x] != '.':
      return self.matrix[point.y][point.x]
    return None


  def bar_graph(self, origin: Point, height: int, value: int, color: any,
                direction: int = -1):
    i = 0
    while i < height:
      if i < value:
        self.set(Point(origin.x, origin.y + direction * i), color)
      else:
        self.set(Point(origin.x, origin.y + direction * i), ' ')
      i += 1


  # returns a value hard-constrained to the square boundary
  def constrain(value):
    if value < 0:
      return 0
    if value > SIDE - 1:
      return SIDE -1 
    return value


  def map_basic(x, in_min, in_max, out_min, out_max):
    """
    Maps a value from one range to another.

    Args:
      x: The input value to be mapped.
      in_min: The minimum value in the input range.
      in_max: The maximum value in the input range.
      out_min: The minimum value in the output range.
      out_max: The maximum value in the output range.

    Returns:
      The mapped value within the output range.
    """

    # Avoid zero division errors
    if in_min == in_max:
      return out_min

    # Perform linear mapping
    slope = (out_max - out_min) / (in_max - in_min)
    mapped_value = slope * (x - in_min) + out_min

    # Clamp output to the defined range
    return max(out_min, min(out_max, mapped_value))

