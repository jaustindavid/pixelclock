from typing import List
from math import sqrt
import random
import defs
from defs import constrain

'''
primitives:
  seek(targets, sandbox)
    try to get to target without stepping on anything in sandbox
    * if a target is adjacent, step there
    * if there are any nearish targets, step towards one
    * wander

'''




""" i have a list of floats; how would I choose from that list, 
    randomly but rpeferring lower numbers? """
def _choose_weighted_random(data):
  """
  Chooses a random element from a list of floats, 
  with a bias towards lower values.

  Args:
      data: A list of floats.

  Returns:
      A randomly chosen element from the list, with 
      higher probability for lower values.
  """
  # Calculate probabilities based on the inverse of the data
  probabilities = [1 / (x + 1) for x in data]

  # Normalize probabilities to sum to 1
  total_probability = sum(probabilities)
  normalized_probabilities = [p / total_probability for p in probabilities]

  # Choose a random element based on the normalized probabilities
  chosen_index = random.choices(range(len(data)), 
                                weights=normalized_probabilities)[0]
  return data[chosen_index]


class Pixel:
  def __init__(self, x=0, y=0, color: str = ' '):
    self.x = constrain(x)
    self.y = constrain(y)
    self.color = color


  def __str__(self):
    return f"({self.x},{self.y}):{self.color}"


  def __eq__(self, other: any):
    return self.x == other.x and self.y == other.y


  # def __hash__(self):
  #   return hash((self.x, self.y, self.color))


  # returns a clone of myself
  def clone(self):
    return Pixel(self.x, self.y, self.color)


  """ Translate self to another pixel """
  def translate(self, dx=0, dy=0, pixel=None):
    if pixel:
      self.x = constrain(self.x + pixel.x)
      self.y = constrain(self.y + pixel.y)
    else:
      self.x = constrain(self.x + dx)
      self.y = constrain(self.y + dy)


  def distance_to(self, other):
    return sqrt((other.x-self.x)**2 + (other.y-self.y)**2)
  

  # returns an other in the same location, or None
  def on_any(self, others) -> any:
    for other in others:
      if self.distance_to(other) == 0:
        return other
    return None


  # returns others, sorted by distance
  def nearest(self, others: List[any]) -> List[any]:
    s = {}
    for other in others:
      s[self.distance_to(other)] = other
    return [value for key, value in sorted(s.items())] 


  # returns one adjacent Pixel, or None
  def adjacent(self, others: List[any]) -> any:
    candidates = []
    for other in others:
      if other is not self \
        and abs(self.x - other.x) <= 1 \
        and abs(self.y - other.y) <= 1:
      # and self.distance_to(other) <= 1.0:
        candidates.append(other)
    if candidates:
      return random.choice(candidates)
    return None


  # returns one near"ish" Pixel, or None
  def nearish(self, others: List[any]) -> any:
    d = {}
    for other in others:
      d[self.distance_to(other)] = other
    if d.keys():
      distance = _choose_weighted_random(list(d.keys()))
      return d[distance]
    return None


  def step(self, dx: int, dy: int, sandbox: List[any]) -> bool:
    p = self.clone()
    p.translate(dx, dy)
    if p not in sandbox:
      self.translate(dx, dy)
      return True
    return False


  # returns a step in the right direction
  # wobble: probability that I might wobble from a straight line
  def _d(self, x, target: int, wobble: float = 0.0):
    if target > x:
      # print(">v")
      weights = [ 0, wobble, 1-wobble ]
    elif target < x:
      # print("<^")
      weights = [ 1-wobble, wobble, 0 ]
    else:
      weights = [ wobble/2, 1-wobble, wobble/2 ]
    return random.choices([-1, 0, 1], weights=weights)[0]


  # returns all others which don't have a friend (same x,y) in sandbox
  def open(self, others, sandbox):
    o = []
    for other in others:
      if not other.on_any(sandbox) and other != self:
        o.append(other)
    return o


  # seek a target which is NOT in sandbox
  # if productive, returns the target (a goal) or True (moved)
  # returns false if blocked
  def seek(self, targets: List[any], sandbox: List[any], 
                 wobble: float = 0.0) -> any:
    # print(f"seek({defs.listr(targets)}, {defs.listr(sandbox)})")
    target = self.adjacent(targets) or self.nearish(targets)
    if target:
      # print(f"{self}: seeking {target}")
      dx = self._d(self.x, target.x, wobble)
      dy = self._d(self.y, target.y, wobble)
      if dx or dy:
        # print(f"{self}: stepping {dx}, {dy}")
        if self.step(dx, dy, sandbox):
          return target
      else:
        # print(f"{self}: non-step: {dx}, {dy}")
        return target
    else:
      return self.wander(sandbox)


  def wander(self, sandbox: List[any]) -> bool:
    for i in range(10):
      dx = random.choice([-1, 0, 1])
      dy = random.choice([-1, 0, 1])
      if self.step(dx, dy, sandbox):
        return True
    return False


class Food(Pixel):
  def __str__(self):
    return f"Food:{super().__str__()}"


  # "==" same as "collides with"
  def __eq__(self, other):
    if super().__eq__(other):
      if isinstance(other, Food):
        return True
    return False


  def __hash__(self):
    return hash(("food", self.x, self.y, self.color))


  def clone(self):
    return Food(self.x, self.y, self.color)


class Ant(Pixel):
  def __str__(self):
    return f"Ant:{super().__str__()}"


  # "==" same as "collides with"
  def __eq__(self, other):
    if super().__eq__(other):
      if not isinstance(other, Food):
        return True
    return False


  def __hash__(self):
    return hash(("ant", self.x, self.y, self.color))


  def clone(self):
    return Ant(self.x, self.y, self.color)


