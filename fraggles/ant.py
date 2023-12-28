'''
an Ant is a mindless drone, just bumps around looking for food
a Queen makes or culls ants based on availability of food
'''

from typing import List
import random
from pixel import Pixel
import defs


class Ant(Pixel):
  def __init__(self):
    super().__init__()
    self.color = 'w'


  def __str__(self):
    return f"Ant" + super().__str__()


  def run(self, food: List[Pixel], sandbox: List[Pixel]):
    if self in food and random.random() < 0.9:
      return
    else:
      # seek food
      available_food = self.open(food, sandbox)
      self.seek(available_food, sandbox) \
          or self.wander(sandbox)


class Queen(Pixel):
  def __init__(self):
    super().__init__()
    self.color = 'g'


  def __str__(self):
    return f"Queen" + super().__str__()


  def eat_ant(self, sandbox: List[Pixel]):
    print("Eating one")
    self.color = 'x'
    ants = [ thing for thing in sandbox if isinstance(thing, Ant) ]
    nom = self.adjacent(ants)
    if nom:
      sandbox.remove(nom)
    else:
      self.seek(ants, sandbox)


  def bear_ant(self, sandbox: List[Pixel]):
    ant = Ant()
    self.color = 'g'
    for dx in [-1, 0, 1]:
      for dy in [-1, 0, 1]:
        ant.x = defs.constrain(self.x+dx)
        ant.y = defs.constrain(self.y+dy)
        if ant not in sandbox:
          sandbox.append(ant)
          return
    # FALLTHROUGH: I must be too close to stuff to bear an ant...
    self.wander()


  def run(self, food: List[Pixel], sandbox: List[Pixel]):
    amt_food = len(food)
    nr_ants = len([thing for thing in sandbox if isinstance(thing, Ant)])
    if nr_ants > amt_food:
      # try to eat an ant
      self.eat_ant(sandbox)
    elif amt_food > nr_ants \
        and random.random() > 0.5:
      # birth an ant
      self.bear_ant(sandbox)
    else:
      self.color = 'q'
      self.wander(sandbox)
