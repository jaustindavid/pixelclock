from typing import List
from datetime import timedelta
import random
import defs
import font
from pixel import Ant, Food
from matrix import Matrix
from timer import Timer
import internet_quality


# might add or subtract ants based on availability of food
def equalize(ants: List[Ant], food: List[Food], sandbox: List[any]):
  if len(ants) > len(food) + 10:
    ant = random.choice(ants)
    ants.remove(ant)
    sandbox.remove(ant)
  elif len(ants) < len(food) - 5:
    ant = Ant(x=defs.SIDE//2, y=defs.SIDE-1, color='w')
    ants.append(ant)
    sandbox.append(ant)


# rolls the dice
def p(f: float) -> float:
  return random.random() < f


# returns a single digit in the time, starting at H H : M M
posn = 0
hhmm = []
def get_next_digit() -> List[Food]:
  global posn, hhmm
  if posn == 0:
     hhmm = font.get_hhmm()
     print(f"HHMM: {hhmm}")
  digit = font.render(hhmm[posn], 4, 3, color='w', scale=1)
  posn += 1
  if posn >= len(hhmm):
    posn = 0
  return digit


if __name__ == "__main__":
  sandbox = []
  ants = []
  food = []
  iq = internet_quality.InternetQuality(sandbox, 15)
  matrix = Matrix(defs.SIDE, sandbox)
  loop = Timer(timedelta(seconds=0.25)) # ratelimit: 1/fps
  showy = Timer(timedelta(seconds=0.5)) # ratelimit: 1/fps
  second = Timer(timedelta(seconds=1))
  food = get_next_digit()
  moves = 0
  frames = 0
  ten_seconds = Timer(timedelta(seconds=10))
  while True:
    if second.expired():
      print(f"{moves} moves, {frames} frames per second")
      print(str(matrix))
      # print(f"graph: {defs.listr(iq.graph)}")
      # print(f"ants: {defs.listr(ants)}")
      # print(f"food: {defs.listr(food)}")
      equalize(ants, food, sandbox)
      moves = 0
      frames = 0
    # iq.run()
    if ten_seconds.expired():
      food = get_next_digit()
    for ant in ants:
      ant.seek(food, sandbox, wobble=0.10)
      moves += 1
    if showy.expired():
      matrix.show()
      frames += 1
    loop.wait()
  print(str(matrix))

iq = None
