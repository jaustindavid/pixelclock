from typing import List
from datetime import datetime
from point import Point
from wobblytime import WobblyTime

FONT = {
  0: '''
 XX 
X  X
X  X
X  X
 XX 
''',
  1: '''
  X
  X
  X
  X
  X
''',
  2: '''
XXX
   X
 XX
X
XXXX
''',
  3: '''
XXX
   X
XXX
   X
XXX
''',
  4: '''
X  X
X  X
XXXX
   X
   X
''',
  5: '''
XXXX
X
XXX
   X
XXX
''',
  6: '''
 XX
X
XXX
X  X
 XX
''',
  7: '''
XXXX
   X
  X
 X
 X
''',
  8: '''
 XX 
X  X
 XX 
X  X
 XX 
''',
  9: '''
 XX
X  X
 XXX
   X
 XX
'''
}


def pstr(points: List[Point]):
  return f"[{','.join([str(p) for p in points])}]"


def decode(symbol: str, dx: int = 0, dy: int = 0) -> List[Point]:
  points = []
  y = 0
  lines = symbol.splitlines()[1:]
  for line in lines:
    x = 0
    for c in line:
      if c == 'X':
        points.append(Point(x+dx,y+dy))
      x += 1
    y += 1
  return points


# returns the set of points representing current hh:mm
_wt = WobblyTime(30,180)
def get_time() -> List[Point]:
  p = list()
  now = datetime.now()
  hh = now.hour
  mm = now.minute
  hh, mm, ss = _wt.gettime()

  h1 = hh // 10
  h2 = hh % 10
  m1 = mm // 10
  m2 = mm % 10
  p.extend(decode(FONT[h1], 3, 2))
  p.extend(decode(FONT[h2], 9, 2))
  p.extend(decode(FONT[m1], 3, 9))
  p.extend(decode(FONT[m2], 9, 9))
  return p
