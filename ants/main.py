from pixel import Ant, Food
from matrix import Matrix
from timer import Timer

if __name__ == "__main__":
  sandbox = []
  food = [ Food(x=8, y=8, color='g') ]
  matrix = Matrix(16, sandbox)
  ant = Ant(color='w')
  sandbox.append(ant)
  print(sandbox[0])
  print(str(matrix))
  ant.step(1,1, sandbox)
  ant.seek(food, sandbox)
  print(str(matrix))

