#!/usr/bin/env python3 
'''
an antfarm which tells the time:
  - a queen just wanders around, either birthing or eating ants
  - each ant just tries to sit on a spot that indicates the time
'''

DEBUG=False
try:
  from machine import Pin
  DEBUG=False
except ModuleNotFoundError as e:
  pass

import random
import defs
from font import get_time
from pixel import Pixel
from matrix import Matrix
from timer import Timer
# TODO: import internet_quality 
from ant import Ant, Queen


'''
a conga line of dots walking right to left to make a graph
''
graph_dots = []
iq_timer = Timer(15)
def graph_iq(iq: internet_quality.InternetQuality, sandbox: List[Pixel]):
  if iq_timer.expired():
    if graph_dots and graph_dots[0].x == 0:
      sandbox.remove(graph_dots[0])
      graph_dots.remove(graph_dots[0])
    for dot in graph_dots:
      dot.step(-1,0, sandbox)
    score = min(iq.age_score(), iq.rtt_score())
    print(f"IQ: {score}, age={iq.age}, rtt={iq.rtt}")
    c = { 4: 'G', 3: 'L', 2: 'Y', 1: 'R' }
    dot = Pixel(defs.SIDE-1, defs.SIDE-1, c[score])
    if dot not in graph_dots:
      graph_dots.append(dot)
      sandbox.append(dot)
'''


if __name__ == "__main__":
  food = get_time()
  sandbox = []
  queen = Queen()
  sandbox.append(queen)
  matrix = Matrix(defs.SIDE, sandbox)
  # TODO iq = internet_quality.InternetQuality(15)
  loop = Timer(0.25) # ratelimit: 4 fps
  second = Timer(1)
  frames = 0
  while True:
    if second.expired():
      queen.run(food, sandbox)
      # TODO iq.run()
      if DEBUG:
        print(f"{frames} frames per second")
        print(str(matrix))
      food = get_time()
      frames = 0
    # TODO graph_iq(iq, sandbox)
    for thing in sandbox:
      if isinstance(thing, Ant):
        thing.run(food, sandbox)
    matrix.show()
    frames += 1
    # loop.wait()
  if DEBUG:
    print(str(matrix))

# TODO iq = None
