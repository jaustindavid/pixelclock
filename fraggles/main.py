from typing import List
from datetime import timedelta
import random
import defs
from font import get_time
from pixel import Pixel
from fraggle import Fraggle
import fraggle as f
from matrix import Matrix
from timer import Timer


# rolls the dice
def p(f: float) -> float:
  return random.random() < f


'''
if there are NO bricks, try to add one in a corner
'''
def manage_bricks(plans: List[Pixel], sandbox: List[Pixel]):
  fraggle = Fraggle()
  bricks = fraggle.find(f.BRICK_COLOR,sandbox)
  print(f"# bricks: {len(bricks)}")
  print(f"plan calls for: {len(plans)}")
  if len(bricks) < len(plans):
    new_brick = random.choice([Pixel(0,0,f.BRICK_COLOR),
                               Pixel(15,0,f.BRICK_COLOR),
                               Pixel(0,15,f.BRICK_COLOR)])
    if new_brick not in sandbox:
      sandbox.append(new_brick)


if __name__ == "__main__":
  fraggles = [Fraggle() for i in range(3)]
  sandbox = []
  sandbox.extend(fraggles)
  plans = []
  matrix = Matrix(defs.SIDE, sandbox)
  loop = Timer(timedelta(seconds=0.25)) # ratelimit: 10 fps
  second = Timer(timedelta(seconds=1))
  plan = get_time()
  frames = 0
  while True:
    if second.expired():
      manage_bricks(plans, sandbox)
      print(f"fraggles: {defs.listr(fraggles)}")
      print(f"plans ({len(plans)}): {defs.listr(plans)}")
      print(f"sandbox ({len(sandbox)}): {defs.listr(sandbox)}")
      print(f"{frames} frames per second")
      print(str(matrix))
      plans = get_time()
      frames = 0
    for fraggle in fraggles:
      fraggle.run(plans, sandbox)
    matrix.show()
    frames += 1
    loop.wait()
  print(str(matrix))

iq = None
