from typing import List
from pixel import Pixel
from fraggle import Fraggle
from matrix import Matrix
import defs
import time


def colorize(n: int) -> str:
  if n < 10:
    return f'{n}'
  return chr(ord('a') + n-10)


def decolor(s: str) -> int:
  if s.isdigit():
    return int(s)
  elif s.islower():
    return ord(s) - ord('a') + 10
  else: 
    return 99


# finds a path in sandbox between two pixels
#
# starting at start:
#   start marked "0"
#   adjacent+open spots are marked 1
#   step to each 1: 
#     adjacent+open spots are marked 2 (if not 1)
# .... repeat
class Pathfinder:
  def __init__(self, sandbox: List[Pixel]):
    self.sandbox = sandbox
    self.grid = []
    self.distances = {}


  # true if pixel + (dx,dy) is in bounds
  def in_bounds(self, pixel: Pixel, dx: int, dy: int) -> bool:
    return pixel.x+dx == defs.constrain(pixel.x+dx) \
            and pixel.y+dy == defs.constrain(pixel.y+dy)


  # returns a list of open pixels in sandbox, adjacent to cursor
  def open_adjacent(self, cursor: Pixel) -> List[Pixel]:
    oa = []
    c = []
    for dx in [-1, 0, 1]:
      for dy in [-1, 0, 1]:
        if not (dx == 0 and dy == 0) and self.in_bounds(cursor, dx, dy):
          p = Pixel(cursor.x+dx, cursor.y+dy)
          c.append(p)
          if p not in self.sandbox:
            oa.append(p)
    return oa


  def get_pixel(self, pixel: Pixel) -> Pixel:
    for p in self.grid:
      if pixel == p:
        return p
    return None


  # sets a "distance" for a pixel
  def set_distance(self, pixel: Pixel, distance: int):
    self.distances[f'{pixel.x},{pixel.y}'] = distance
    p = Pixel(pixel.x, pixel.y, colorize(distance))
    if p in self.grid:
      self.get_pixel(p).color = p.color
    else:
      self.grid.append(p)


  # returns the "distance" to a pixel, previously set
  def get_distance(self, pixel: Pixel) -> int:
    key = f'{pixel.x},{pixel.y}'
    if key in self.distances:
      return self.distances[key]
    return 999


  def get_pixels(self, distance: int) -> List[Pixel]:
    pixels = []
    for xy in self.distances.keys():
      if self.distances[xy] == distance:
        x, y = map(int, xy.split(','))
        pixels.append(Pixel(x,y))
    return pixels


  # attempts to run ONE MORE STEP toward dest
  # True if gets there
  # leaves a breadcrumb along the way
  def run2(self, cursor: Pixel, dest: Pixel, 
          distance: int, debug: bool = False) -> bool:
    if not distance:
      return False
    maybe = False
    if debug: print(f"running {cursor} --{distance}--> {dest}")
    neighbors = self.open_adjacent(cursor)
    if debug: print(f"neighbors: {defs.listr(neighbors)}")
    if dest in neighbors:
      if debug: print("YESSSS")
      return True
    for neighbor in neighbors:
      # print(f"{cursor}: smelling {neighbor}")
      if self.get_distance(neighbor) < self.get_distance(cursor):
        if debug: print(f"{cursor} -> {neighbor}: downhill, skipping")
        continue
      elif self.get_distance(neighbor) == 999:
        if debug: print(f"{cursor} -> {neighbor}: new, adding one")
        self.set_distance(neighbor, self.get_distance(cursor)+1)
      elif self.get_distance(neighbor) > self.get_distance(cursor)+2:
        if debug: print(f"{cursor} -> {neighbor}: better path, recording")
        self.set_distance(neighbor, self.get_distance(cursor)+1)
      else:
        if debug: print(f"{cursor}->{neighbor} is the way")
        if self.run(neighbor, dest, distance-1):
          return True
    return False


  def run(self, source: Pixel, dest: Pixel, 
          distance: int, debug: bool = False) -> bool:
    if not distance:
      self.set_distance(source, 0)
    if debug: print(f"running {source} --{distance}--> {dest}")
    targets = self.get_pixels(distance-1)
    if debug:
      print(f"targets @ {distance}: {defs.listr(targets)}")
    for cursor in targets:
      # test adjacent
      neighbors = self.open_adjacent(cursor)
      print(f"neighbors: {defs.listr(neighbors)}")
      if dest in neighbors:
        if debug: print("YESSSS")
        self.set_distance(dest, distance)
        return True
      for neighbor in neighbors:
        if self.get_distance(neighbor) > distance:
          self.set_distance(neighbor, distance)
    return False


  def backtrace(self, dest: Pixel, source: Pixel):
    path = []
    cursor = dest.clone()
    while cursor != source:
      neighbors = self.open_adjacent(cursor)
      for neighbor in neighbors:
        if self.get_distance(neighbor) < self.get_distance(cursor):
          path.insert(0, cursor)
          cursor = neighbor
          break
    return path


  def navigate(self, source: Pixel, dest: Pixel,
               debug: bool = False) -> List[Pixel]:
    self.grid = []
    self.distances = {}
    self.grid.extend(self.sandbox)
    print(f"navigating from {source} -> {dest}")
    self.set_distance(source, 0)
    path = []
    for distance in range(15):
      # print(f"Sandbox:")
      # print(Matrix.to_str(16, self.sandbox))
      # print(f"running {distance}")
      if self.run(source, dest, distance, debug=debug):
        print(f"FOUND IT!!!")
        path = self.backtrace(dest, source)
        break
      if debug:
        print(Matrix.to_str(16, self.grid))
    if path:
      print(f"solution: {source} -> {defs.listr(path)} -> {dest}")
    return path


if __name__ == "__main__":
  sandbox = [ Pixel(x,x,'X') for x in range(2,6) ]
  sandbox.extend( [ Pixel(x+1,x,'X') for x in range(2,6) ])
  fraggles = [ Fraggle(), Fraggle() ]
  sandbox.extend(fraggles)
  print(defs.listr(sandbox))
  f = Pixel()
  while f in sandbox:
    for fraggle in fraggles:
      fraggle.color = 'F'
      fraggle.wander(sandbox)
  print(Matrix.to_str(16, sandbox))
  pathfinder = Pathfinder(sandbox)
  pathfinder.navigate(Pixel(2,3), Pixel(4,2), debug=True)
  pathfinder.navigate(Pixel(0,3), Pixel(6,2), debug=True)
  pathfinder.navigate(Pixel(0,3), Pixel(14,2), debug=True)
