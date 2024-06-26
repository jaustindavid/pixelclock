from typing import List, Tuple
from pixel import Pixel
from colors import COLOR

CLI_MODE = False
try:
  import neopixel
  import board
except ModuleNotFoundError as e:
  print("Failed to import board OR neopixel; running CLI-only")
  CLI_MODE = True

"""
The Matrix just ("just") paints all the Pixels in a list.  It maintains
an internal buffer and only re-paints the changes.

in CLI_MODE it does everything EXCEPT for instnatiating the NeoPixel
library, or using it.  But internal buffers are still maintained

"""


class Matrix:
  def __init__(self, size: int, sandbox: List):
    self.size = size
    self.sandbox = sandbox
    self.buffer = self.fill(COLOR[' ']) 
    if not CLI_MODE:
      self.pixels = neopixel.NeoPixel(board.D18, self.size * self.size,
                                      auto_write=False)
      self.pixels.fill(COLOR[' '])


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


  def fill(self, color: Tuple[int,int,int]) -> List[Tuple[int,int,int]]:
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
    new_buffer = self.fill(COLOR[' '])
    for item in self.sandbox:
      new_buffer[self._coord(item)] = COLOR[item.color]
    for coord, color in enumerate(new_buffer):
      if self.buffer[coord] != new_buffer[coord]:
        self.buffer[coord] = new_buffer[coord]
        if not CLI_MODE:
          self.pixels[coord] = new_buffer[coord]
    if not CLI_MODE:
      self.pixels.show()


if __name__ == "__main__":
  m = Matrix(16, [])
  print(str(m))
