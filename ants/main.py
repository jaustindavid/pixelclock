from typing import List
from datetime import timedelta
import random
import defs
from font import get_time
from pixel import Ant, Food
from matrix import Matrix
from timer import Timer
import internet_quality


# might add or subtract ants based on availability of food
def equalize(ants: List[Ant], food: List[Food], sandbox: List[any]):
  if len(ants) > len(food):
    ant = random.choice(ants)
    ants.remove(ant)
    sandbox.remove(ant)
  elif len(ants) < len(food):
    ant = Ant(x=defs.SIDE//2, y=defs.SIDE-1, color='w')
    ants.append(ant)
    sandbox.append(ant)


# rolls the dice
def p(f: float) -> float:
  return random.random() < f


if __name__ == "__main__":
  ants = []
  sandbox = []
  food = []
  iq = internet_quality.InternetQuality(sandbox, 15)
  matrix = Matrix(defs.SIDE, sandbox)
  loop = Timer(timedelta(seconds=0.1)) # ratelimit: 10 fps
  second = Timer(timedelta(seconds=1))
  food = get_time()
  moves = 0
  frames = 0
  while True:
    if second.expired():
      print(f"{moves} moves, {frames} frames per second")
      print(str(matrix))
      print(f"graph: {defs.listr(iq.graph)}")
      print(f"ants: {defs.listr(ants)}")
      print(f"food: {defs.listr(food)}")
      food = get_time()
      equalize(ants, food, sandbox)
      moves = 0
      frames = 0
    iq.run()
    for ant in ants:
      if ant.on_any(food) and p(0.95):
        # 95% chance of staying on food
        continue
      else:
        ant.seek(food, sandbox, wobble=0.05)
        moves += 1
    matrix.show()
    frames += 1
    loop.wait()
  print(str(matrix))

iq = None
