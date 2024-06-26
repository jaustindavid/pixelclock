from pixel import Pixel
from colors import COLOR
from timer import Timer
import defs


try:
  from machine import Pin
  import neopixel
  # TODO import adafruit_bh1750
  CLI_MODE = False
except ModuleNotFoundError as e:
  print("Failed to import machine OR neopixel; running CLI-only")
  CLI_MODE = True

"""
The Matrix just ("just") paints all the Pixels in a list.  It maintains
an internal buffer and only re-paints the changes.

in CLI_MODE it does everything EXCEPT for instnatiating the NeoPixel
library, or using it.  But internal buffers are still maintained

"""


AUTO_WRITE=False
class Matrix:
  def __init__(self, size: int, sandbox: list[Pixel]):
    self.size = size
    self.sandbox = sandbox
    self.buffer = self.fill(COLOR[' ']) 
    self.sensor = None
    self.brightness_timer = Timer(5)
    self.brightness_timer.expire()
    self.last_brightness = -1
    self.pixels = None
    if not CLI_MODE:
      try: 
        pin = Pin(13, Pin.OUT)
        self.pixels = neopixel.NeoPixel(pin, self.size * self.size)
        self.pixels.fill(COLOR[' '])
      except RuntimeError:
        print("failed to instantiate NeoPixel.  No pixels, next time be root")
      # TODO
      # try:
      #   i2c = board.I2C()
      #   self.sensor = adafruit_bh1750.BH1750(i2c)
      # except Exception:
      #   print("failed to pull I2C; no brightness control")
      # self.set_brightness()


  def __str__(self):
    buffer = [ [ ' ' for _ in range(self.size) ] for _ in range(self.size) ]
    # TODO: handle overlapping items, like ants & food
    for item in self.sandbox:
      if buffer[item.y][item.x] == ' ':
        buffer[item.y][item.x] = item.color

    ret = ""
    for row in buffer:
      ret += f" {' '.join(row)}\n"
    return ret


  # True if x = y "ish"
  def ish(x: float, y: float, var: float): 
    tolerance = abs(y*var)
    return abs(x-y) <= tolerance


  def set_brightness(self):
    if not CLI_MODE and self.pixels and self.sensor:
      brightness = defs.map_basic(self.sensor.lux, 0, 200, 0.1, 1.0)
      # print(f"{self.last_brightness} vs. new {brightness}: {Matrix.ish(brightness, self.last_brightness, 0.1)}")
      # if brightness != self.last_brightness \
      if not Matrix.ish(brightness, self.last_brightness, 0.1) \
          and self.brightness_timer.expired():
        self.pixels.brightness = brightness
        self.last_brightness = brightness


  # color is a tuple (r, g, b)
  # returns a list of those
  def fill(self, color: any) -> list[any]:
    return [color for i in range(self.size * self.size)]


  # translate a Pixel (x,y) to a 
  def _coord(self, pixel: Pixel) -> int:
    if pixel.x % 2 == 0:
      p = pixel.x * self.size + pixel.y
    else:
      p = pixel.x * self.size + self.size - pixel.y - 1
    return p


  # udpates self.buffer from self.sandbox
  # then pushes and changed Pixels to self.pixels
  def show(self):
    if self.pixels:
      self.set_brightness()
      new_buffer = self.fill(COLOR[' '])
      for item in self.sandbox:
        new_buffer[self._coord(item)] = COLOR[item.color]
      for coord, color in enumerate(new_buffer):
        if self.buffer[coord] != new_buffer[coord]:
          self.buffer[coord] = new_buffer[coord]
          if not CLI_MODE:
            self.pixels[coord] = new_buffer[coord]
      if not AUTO_WRITE:
        self.pixels.write()


  def to_str(size: int, sandbox: list[Pixel]):
    buffer = [ [ ' ' for _ in range(size) ] for _ in range(size) ]
    for item in sandbox:
      if buffer[item.y][item.x] == ' ':
        buffer[item.y][item.x] = item.color

    ret = ""
    for row in buffer:
      ret += f" {' '.join(row)}\n"
    return ret


if __name__ == "__main__":
  for x, y in [(-1, 1), (1, 0.9), (1, 0.8), (-1, 0.9), (1, 1.1), (1,1)]:
    print(f"{x} <> {y}: {Matrix.ish(x, y, 0.1)}")
  import time
  sandbox = []
  m = Matrix(16, sandbox) 
  x = 0
  y = 0
  for color in COLOR.keys():
    sandbox.append(Pixel(x,y,color))
    sandbox.append(Pixel(defs.SIDE-x-1, defs.SIDE-y-1,color))
    x += 1
    if x >= defs.SIDE:
      x = 0
      y += 1
  print(str(m))
  if m.sensor:
    print("running forever, play with the light sensor")
    while True:
      m.show()
      time.sleep(0.5)
