from typing import List, Tuple
from datetime import datetime
from pixel import Pixel
from wobblytime import WobblyTime
import defs

# huge: 8x8
FONT = {
  0: '''\
  XXXX
 X    X
X      X
X      X
X      X
X      X
 X    X
  XXXX
''',
  1: '''\
   XX
   XX
   XX
   XX
   XX
   XX
   XX
   XX
''',
  2: '''\
 XXXXXX
X      X
       X       
     X
   X
 X
X
XXXXXXXX
''',
  3: '''\
XXXXXXXX
      X
    X
  XXXXX
       X
       X
X      X
 XXXXXX
''',
  4: '''\
 X    X
 X    X
 X    X
 XXXXXXX
      X
      X
      X
      X
''',
  5: '''\
XXXXXXXX
X
X
XXXXXXX
       X
       X
X      X
 XXXXXX
''',
  6: '''\
  XXXXX
 X     X
 X     
 X XXXX
 X     X
 X     X
 X     X
  XXXXX
''',
  7: '''\
XXXXXXXX
X      X
      X
     X
    X
    X
    X
    X
''',
  8: '''\
  XXXXX
 X     X
 X     X
  XXXXX
 X     X
 X     X
 X     X
  XXXXX
''',
  9: '''\
  XXXXX
 X     X
 X     X
  XXXX X
       X
       X
 X     X
  XXXXX
''',
  ':': '''\
   XX
   XX
   XX


   XX
   XX
   XX
'''
}


# decodes a character into a list of Pixels, potentially offset
def decode(symbol: str, dx: int = 0, dy: int = 0) -> List[Pixel]:
  pixels = []
  y = 0
  lines = symbol.splitlines()
  for line in lines:
    x = 0
    for c in line:
      if c == 'X':
        pixels.append(Pixel(x+dx,y+dy))
      x += 1
    y += 1
  return pixels


# renders a symbol with a scale multiplier
def render(symbol: any, dx: int = 0, dy: int = 0, 
           color: str = ' ', scale: int = 1) -> List[Pixel]:
  pixels = []
  y = 0
  lines = FONT[symbol].splitlines()
  for line in lines:
    for l in range(scale):
      x = 0
      for c in line:
        for j in range(scale):
          if c == 'X':
            pixels.append(Pixel(x+dx, y+dy, color=color))
          x += 1
      y += 1
  return pixels


def get_hhmm() -> Tuple[int]:
  hh, mm, ss = _wt.gettime()
  return (hh//10, hh%10, ':', mm//10, mm%10)


# returns the set of points representing current hh:mm
_wt = WobblyTime(30,180)
def get_time() -> List[Pixel]:
  pixels = []
  now = datetime.now()
  hh = now.hour
  mm = now.minute
  hh, mm, ss = _wt.gettime()

  h1 = hh // 10
  h2 = hh % 10
  m1 = mm // 10
  m2 = mm % 10
  pixels.extend(decode(FONT[h1], 3, 2))
  pixels.extend(decode(FONT[h2], 9, 2))
  pixels.extend(decode(FONT[m1], 3, 9))
  pixels.extend(decode(FONT[m2], 9, 9))
  return pixels
