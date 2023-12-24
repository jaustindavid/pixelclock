from typing import List
import time
import random
import defs
from pixel import Pixel

'''
A Fraggle is a Pixel which can retrieve a thing from a spot, 
then move it to another location.  It's a builder.  it has 
intent and state.

RESTING: looking for something to do; trying not to get in the way 
FETCHING: hunting for a new brick (dumping spot or not in plan)
BUILDING: taking the new brick to a free spot on the plan
CLEANING: taking a wild brick to a dumping spot


needs:
  "find a brick not in the plan"
    an item of a color in sandbox which isn't represented in plan
    * item color = BRICK_COLOR; in sandbox, not in plan
    * same as item in sandbox not in plan AND color = BRICK_COLOR
      pixel.find(pixel.open(sandbox, plan), color=BRICK_COLOR)
  "find a plan without a brick"
    an item in plan without a color in sandbox
    * item color = PLAN_COLOR, in plan, not in sandbox
    * same as item in plan not in sandbox : pixel.open(plan, sandbox)

'''

# states
RESTING = 0
FETCHING = 1
BUILDING = 2
CLEANING = 3
DUMPING = 4
STUCK = 5

# constants
BRICK_COLOR = 'r'

colors = {
    RESTING:  'w',
    FETCHING: 'b',
    BUILDING: 'B',
    CLEANING: 'g',
    DUMPING:  'G',
    STUCK:    'h'
}

class Fraggle(Pixel):
  def __init__(self):
    super().__init__()
    self.state = RESTING
    self.last_loc = Pixel(self.x, self.y)
    self.stuckness = 0


  def __str__(self):
    return f"Fraggle:{super().__str__()} state:{self.state}"

  # returns a list of pixels with a specified color
  def find(self, color: str, haystack: List[Pixel]) -> List[Pixel]:
    candidates = []
    for needle in haystack:
      if needle.color == color:
        candidates.append(needle)
    return candidates


  '''
  looking for something to do
  - if there are any plan spots without a brick, FETCHING
  - if there are any bricks not in the plan, switch to CLEANING
  - if I'm adjacent to anything, move to an open spot
  '''
  def rest(self, plans: List[Pixel], sandbox: List[Pixel]):
    if self.open(plans, sandbox):
      self.state = FETCHING
    elif self.find(BRICK_COLOR, self.open(sandbox, plans)):
      self.state = CLEANING
    elif self.adjacent(sandbox):
      self.wander(sandbox)
    else:
      print(f"{self} resting")


  '''
  walk to a brick in the sandbox which is not in the plans,
  pick it up, set state to BUILDING
  '''
  def fetch(self, plans: List[Pixel], sandbox: List[Pixel]):
    # if I'm next to a brick, pick it up & build
    bricks = self.find(BRICK_COLOR, self.open(sandbox, plans))
    brick = self.adjacent(bricks)

    if brick:
      sandbox.remove(brick)
      self.state = BUILDING
    else:
      # otherwise, keep seeking that brick
      # print(f"{self} Fetching: {defs.listr(bricks)}")
      self.seek(bricks, sandbox, wobble=0.0) \
        or self.seek(bricks, sandbox, wobble=0.10) \
        or self.seek(bricks, sandbox, wobble=0.50) 
      self.am_i_stuck()


  def fseek(self, plans, sandbox, wobble):
    if self.adjacent(sandbox):
      print(f"{self} seeking")
      print(f"sandbox: {defs.listr(sandbox)}")
    super().seek(plans, sandbox, wobble)


  def on_any(self, sandbox: List[Pixel]):
    return [ p for p in sandbox if (p is not self and p == self) ]


  '''
  walk to a spot in the plan, drop a brick, set state to RESTING
  '''
  def build(self, plans: List[Pixel], sandbox: List[Pixel]):
    # if I'm on a plan spot, drop my brick
    if self.on_any(plans) and not self.on_any(sandbox):
      brick = Pixel(self.x, self.y, BRICK_COLOR)
      sandbox.append(brick)
      self.state = RESTING
    else:
      # walk to an open spot
      if self.open(plans, sandbox):
        self.seek(self.open(plans, sandbox), sandbox, wobble=0.0) \
          or self.seek(self.open(plans, sandbox), sandbox, wobble=0.10) \
          or self.seek(self.open(plans, sandbox), sandbox, wobble=0.50)
        self.am_i_stuck()
      else:
        self.state = RESTING


  '''
  walk to a loose brick, pick it up, dump it
  almost identical to fetch()
  '''
  def clean(self, plans: List[Pixel], sandbox: List[Pixel]):
    # if I'm next to a brick, pick it up & build -- same as fetch
    bricks = self.find(BRICK_COLOR, self.open(sandbox, plans))
    if bricks:
      brick = self.adjacent(bricks)
      if brick:
        sandbox.remove(brick)
        self.state = DUMPING
      else:
        # otherwise, keep seeking that brick
        self.seek(bricks, sandbox, wobble=0.10)
        self.am_i_stuck()
    else:
      self.state = RESTING


  '''
  almost identical to build()
  '''
  def dump(self, plans: List[Pixel], sandbox: List[Pixel]):
    # if I'm on THE dump spot, drop my brick
    if self == Pixel(15, 15):
      # brick = Pixel(self.x, self.y, BRICK_COLOR)
      # sandbox.append(brick)
      # actually it's just gone
      self.state = RESTING
    else:
      # walk to the dumping spot
      self.seek([Pixel(15,15)], sandbox, wobble=0.10)
      self.am_i_stuck()

  
  def am_i_stuck(self):
    if self.last_loc == self:
      self.stuckness += 0
      if self.stuckness >= 10:
        self.pre_stuck_state = self.state
        self.state = STUCK
        self.sticky_counter = 10
    else:
      self.stuckness = 0
      self.last_loc = Pixel(self.x, self.y)


  '''
  I think I got stuck!  wander around for a while
  '''
  def unstick(self, plans: List[Pixel], sandbox: List[Pixel]):
    if self.sticky_counter:
      self.sticky_counter -= 1
      self.wander()
    else:
      self.state == self.pre_stuck_state


  # manages states, but NOT state transitions
  def run(self, plans: List[Pixel], sandbox: List[Pixel]):
    if self.state == STUCK:
      self.unstick(plans, sandbox)
    elif self.state == FETCHING:
      self.fetch(plans, sandbox)
    elif self.state == BUILDING:
      self.build(plans, sandbox)
    elif self.state == CLEANING:
      self.clean(plans, sandbox)
    elif self.state == DUMPING:
      self.dump(plans, sandbox)
    else:
      self.state = RESTING # just in case
      self.rest(plans, sandbox)
    self.color = colors[self.state]
