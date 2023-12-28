from typing import List
from datetime import datetime
from pixel import Pixel
from wobblytime import WobblyTime
import defs


FONT = {
  0: '''\
 XX 
X  X
X  X
X  X
 XX 
''',
  1: '''\
  X
  X
  X
  X
  X
''',
  2: '''\
XXX
   X
 XX
X
XXXX
''',
  3: '''\
XXX
   X
XXX
   X
XXX
''',
  4: '''\
X  X
X  X
XXXX
   X
   X
''',
  5: '''\
XXXX
X
XXX
   X
XXX
''',
  6: '''\
 XX
X
XXX
X  X
 XX
''',
  7: '''\
XXXX
   X
  X
 X
 X
''',
  8: '''\
 XX 
X  X
 XX 
X  X
 XX 
''',
  9: '''\
 XX
X  X
 XXX
   X
 XX
'''
}


# decodes a character into a list of Pixels, potentially offset
def decode(symbol: str, dx: int = 0, dy: int = 0,
           color: str = ' ') -> List[Pixel]:
  pixels = []
  y = 0
  lines = symbol.splitlines()
  for line in lines:
    x = 0
    for c in line:
      if c == 'X':
        pixels.append(Pixel(x+dx, y+dy, color))
      x += 1
    y += 1
  return pixels


# returns the set of points representing current hh:mm
_wt = WobblyTime(30,180)
def get_time(color: str = ' ') -> List[Pixel]:
  pixels = []
  now = datetime.now()
  hh = now.hour
  mm = now.minute
  hh, mm, ss = _wt.gettime()

  h1 = hh // 10
  h2 = hh % 10
  m1 = mm // 10
  m2 = mm % 10
  pixels.extend(decode(FONT[h1], 3, 1))
  pixels.extend(decode(FONT[h2], 9, 1))
  pixels.extend(decode(FONT[m1], 3, 8))
  pixels.extend(decode(FONT[m2], 9, 8))
  return pixels
