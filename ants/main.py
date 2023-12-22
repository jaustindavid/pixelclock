from datetime import timedelta
import defs
from font import get_time
from pixel import Ant, Food
from matrix import Matrix
from timer import Timer
import internet_quality


if __name__ == "__main__":
  sandbox = []
  iq = internet_quality.InternetQuality(sandbox, 15)
  food = [ Food(x=8, y=8, color='g') ]
  matrix = Matrix(defs.SIDE, sandbox)
  ant = Ant(color='w')
  sandbox.append(ant)
  print(sandbox[0])
  loop = Timer(timedelta(seconds=0.25))
  second = Timer(timedelta(seconds=1))
  while ant.distance_to(food[0]) > 0:
    if second.expired():
      food = get_time()
    iq.run()
    print(str(matrix))
    print(ant)
    print(food[0])
    ant.seek(food, sandbox, 0.05)
    defs.print_list(sandbox)
    loop.wait()
  print(str(matrix))

iq = None
