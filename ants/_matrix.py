#!/usr/bin/env python

from time import sleep
import random

CLI_MODE = False
try:
  import board
  import neopixel
except ModuleNotFoundError as e:
  print("Failed to import board OR neopixel; running CLI-only")
  CLI_MODE = True

from colors import COLOR
from pixel import Pixel


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


  def set(self, pixel, color):
    # translates Pixel to a pixel location
    def _pixel() -> int:
      # print(f"Trying {pixel}: x={pixel.x}, y={pixel.y}")
      if pixel.x % 2 == 0:
        p = pixel.x * SIDE + pixel.y
      else:
        p = pixel.x * SIDE + SIDE - pixel.y - 1
      # print(f"returning {p}")
      return p

    self.matrix[pixel.y][pixel.x] = color
    if color in COLOR:
      rgb = COLOR[color]
    else:
      print(f"WARNING: color '{color}' not defined")
      rgb = (0,0,0)
    if not CLI_MODE:
      if self.pixels[_pixel()] != rgb:
        self.pixels[_pixel()] = rgb


  def unset(self, pixel):
    self.set(pixel, ' ')


  def get(self, pixel):
    if self.matrix[pixel.y][pixel.x] != ' ':
      return self.matrix[pixel.y][pixel.x]
    return None


  def bar_graph(self, origin: Pixel, height: int, value: int, color: any,
                direction: int = -1):
    i = 0
    while i < height:
      if i < value:
        self.set(Pixel(origin.x, origin.y + direction * i), color)
      else:
        self.set(Pixel(origin.x, origin.y + direction * i), ' ')
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

