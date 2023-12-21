from typing import List
from pixel import Pixel

CLI_MODE = False
try:
  import board
  import neopixel
except ModuleNotFoundError as e:
  print("Failed to import board OR neopixel; running CLI-only")
  CLI_MODE = True

class Matrix:
  def __init__(self, size: int, sandbox: List):
    self.size = size
    self.sandbox = sandbox
    if not CLI_MODE:
      self.pixels = neopixel.NeoPixel(board.D18, self.size * self.size)
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



if __name__ == "__main__":
  m = Matrix(16, [])
  print(str(m))
