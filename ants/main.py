#!/usr/bin/env python

from typing import List
import sys

from time import sleep
from datetime import datetime, timedelta
import random

from point import Point
from matrix import Matrix, SIDE
from ant import Ant
from font import FONT, pstr, decode, get_time
from timer import Timer

from math import sqrt



""" i have a list of floats; how would I choose from that list, randomly but rpeferring lower numbers? """
def choose_weighted_random(data):
  """
  Chooses a random element from a list of floats, with a bias towards lower values.

  Args:
      data: A list of floats.

  Returns:
      A randomly chosen element from the list, with higher probability for lower values.
  """
  # Calculate probabilities based on the inverse of the data
  probabilities = [1 / (x + 1) for x in data]

  # Normalize probabilities to sum to 1
  total_probability = sum(probabilities)
  normalized_probabilities = [p / total_probability for p in probabilities]

  # Choose a random element based on the normalized probabilities
  chosen_index = random.choices(range(len(data)), weights=normalized_probabilities)[0]

  return data[chosen_index]


''' returns the linear distance (float) from p1(x,y) to p2(x, y) '''
def distance(p1, p2):
  return sqrt((p2.x-p1.x)**2 + (p2.y - p1.y)**2)


'''
returns the nearest pheromone (x,y) to ant
'''
def nearish(ant, pheromones):
  dists = {}
  for p in pheromones:
    # print(f"{ant} <=> {p} = {distance(ant.point, p)}")
    # 80% of staying on top of a pheromone
    if distance(ant.point, p) == 0:
      print("0!")
      if random.random() < 0.9:
        print("90%!")
        return p
    dists[distance(ant.point, p)] = p
  if dists.keys():
    d = choose_weighted_random(list(dists.keys()))
    return dists[d]
  return None


# returns any element in list
def any_of(points: List[Point]):
  if points:
    return random.choice(points)
  return []


# returns any point in targets with distance < 1.5
def adjacent_cells(source: Point, targets: List[Point]):
  return [ target for target in targets if distance(source, target) < 1.5 ]


# true if all ants are NOT on point
def open_cell(point: Point, ants: List[Ant]):
  # for ant in ants:
  #   print(f"Ant: {ant} vs {point}")
  return all([ ant.point != point for ant in ants ])


# returns any points which aren't occupied by ants
def open_cells(points: List[Point], ants: List[Ant]):
  return [ point for point in points if open_cell(point, ants) ]


  


m = Matrix()
ants = []
pheromones = []
sec = Timer(timedelta(seconds=2))
loop = Timer(timedelta(seconds=0.25))
while True:
  old_pheromones = pheromones
  pheromones = get_time()
  if sec.expired():
    if len(ants) > len(pheromones):
      ant = ants.pop()
    elif len(ants) < len(pheromones):
      ants.append(Ant(m, 'o', point=Point(SIDE//2,SIDE//2)))
   
  for p in pheromones:
    if p not in old_pheromones and not m.get(p):
     m.set(p, '.')
  for ant in ants:
    if ant.point in pheromones \
        and random.random() < 0.9:
      continue
    op = open_cells(pheromones, ants)
    adj = adjacent_cells(ant.point, open_cells(pheromones, ants))
    if ant.color == 'D':
      print(f"open cells: {pstr(op)}")
      print(f"adjacent cells: {pstr(adj)}")
    if not adj:
      noc = nearish(ant, open_cells(pheromones, ants))
      if ant.color == 'D':
        if noc:
          print(f"nearish open cells: {pstr(noc)}")
        else:
          print(f"no nearish open cells")
    pt = any_of(adjacent_cells(ant.point, open_cells(pheromones, ants))) \
         or nearish(ant, open_cells(pheromones, ants))
    if pt:
      if ant.color == 'D':
        print(f"{ant} -> {pt}")
      ant.walkTo(targetPoint=pt, wobble=0.00)
    else:
      ant.wander()
    if ant.point in pheromones:
      m.set(ant.point, m.get(ant.point).upper())
  # for p in pheromones:
  #   if m.get(p):
  #     m.set(p, m.get(p).upper())
  #   else:
  #     pass
  #     # m.set(p, '.')
  m.show()
  loop.wait()
  # for p in pheromones:
  #   if not m.get(p):
  #     m.unset(p)
