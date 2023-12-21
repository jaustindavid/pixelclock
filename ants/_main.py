#!/usr/bin/env python

import sys
from typing import List
from math import sqrt

from time import sleep
from datetime import datetime, timedelta
import random

from point import Point
from matrix import Matrix, SIDE
from ant import Ant
import font
from font import FONT, pstr, decode, get_time
from timer import Timer
import ping
from ping import ping_forever
from mproc import ForeverProcess



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
returns the nearest food (x,y) to ant
'''
def nearish(ant, food):
  dists = {}
  for p in food:
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


# displays a graph showing internet quality
# can be run FREQUENTLY without hosing everything up
class InternetQuality: 
  def __init__(self, matrix: Matrix, interval: timedelta):
    self.matrix = matrix
    self.timer = Timer(interval)
    self.graph = []
    self.age = 999
    self.rtt = 999
    self._read_ping_data()


  # returns 1..4 stars
  def _rtt_score(self):
    return 5 - int(Matrix.map_basic(self.rtt, 1, 500, 1, 4))


  # returns 1..4 stars
  def _age_score(self):
    return 5 - int(Matrix.map_basic(self.age, 0, 60, 1, 4))


  def _read_ping_data(self):
    (rtt, latency_score, latency_timestamp) = ping.get_score()
    self.age = datetime.now().timestamp() - latency_timestamp
    self.rtt = rtt
    print(f"ping data: score={latency_score}, rtt={rtt}, age: {self.age}")


  def run(self): 
    c = {
      1: 'R',
      2: 'Y',
      3: 'L',
      4: 'G'
    }
    if self.timer.expired():
      self._read_ping_data()
      for ant in self.graph:
        if ant.point.x == 0:
          self.graph.remove(ant)
          ant.immolate()
        else:
          ant.step(-1, 0)
      if not self.matrix.get(Point(SIDE-1, SIDE-1)):
        self.graph.append(Ant(matrix=self.matrix, 
                              color=c[self._rtt_score()],
                              point=Point(SIDE-1, SIDE-1)))
      print(f"Graph: {len(self.graph)} ants")


  def test_pattern(self):
    self.matrix.set(Point(0,0), 'G')
    self.matrix.set(Point(0,1), 'L')
    self.matrix.set(Point(0,2), 'Y')
    self.matrix.set(Point(0,3), 'R')
    sleep(5)
    self.matrix.unset(Point(0,0))
    self.matrix.unset(Point(0,1))
    self.matrix.unset(Point(0,2))
    self.matrix.unset(Point(0,3))


if __name__ == "__main__":
  pinger = ForeverProcess(ping_forever)
  pinger.start()

  m = Matrix()
  iq = InternetQuality(m, timedelta(seconds=15))
  # iq.test_pattern()
  ants = []
  food = []
  sec = Timer(timedelta(seconds=1))
  loop = Timer(timedelta(seconds=0.25))
  latency_score = 0
  latency_age = 0
  while True:
    iq.run()
    old_food = food
    food = get_time()
    if sec.expired():
      if len(ants) > len(food):
        ant = ants.pop()
      elif len(ants) < len(food):
        ants.append(Ant(m, 'o', point=Point(SIDE//2,SIDE-1)))
      m.show()
    for p in food:
      if p not in old_food and not m.get(p):
       m.set(p, '.')
    for ant in ants:
      # 90% chance to do nothing if on food
      if ant.point in food \
          and random.random() < 0.95:
        continue
      pt = any_of(adjacent_cells(ant.point, open_cells(food, ants))) \
             or nearish(ant, open_cells(food, ants))
      if pt:
        if ant.color == 'D':
          print(f"{ant} -> {pt}")
        ant.walkTo(targetPoint=pt, wobble=0.00)
      else:
        ant.wander()
      if ant.point in food:
        m.set(ant.point, m.get(ant.point).upper())
    loop.wait()
