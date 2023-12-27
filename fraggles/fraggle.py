from typing import List
import time
from datetime import timedelta
import random
import defs
from pixel import Pixel
from timer import Timer

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
STUCK_COUNTER = 10 # times without moving before being "stuck"
UNSTUCK_DIST = 3 # distance to wander before being "unstuck"

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
    self.work_timer = None
    self.last_state = 9 # no state
    self.dumps = [ [Pixel(x, defs.SIDE-3) for x in range(defs.SIDE)],
                   [Pixel(x, defs.SIDE-2) for x in range(defs.SIDE)] ]


  def __str__(self):
    return f"Fraggle:{super().__str__()} state:{self.state}"


  # returns a list of pixels with a specified color
  def find(self, color: str, haystack: List[Pixel]) -> List[Pixel]:
    candidates = []
    for needle in haystack:
      if needle.color == color:
        candidates.append(needle)
    return candidates


  # returns the haystack without needles
  def subtract(self, haystack: List[Pixel], 
                     needles: List[Pixel]) -> List[Pixel]:
    return [ hay for hay in haystack if hay not in needles ]


  # returns a from self to dest, navigating sandbox
  def _path_to(self, dest: Pixel, depth: int, 
                    sandbox: List[Pixel],
                    debug: bool = False) -> List[Pixel]:
    if debug: 
      defs.debug(f"Trying {self} = {depth} => {dest}")
      defs.debug(defs.listr(sandbox))

    if self.adjacent([dest]):
      return [Pixel(self.x, self.y), dest]
    if depth == 1:
      return None
    possibles = []
    for x in -1, 0, 1:
      for y in -1, 0, 1:
        fraggle = Fraggle()
        if (self.x + x) == defs.constrain(self.x + x) \
            and (self.y + y) == defs.constrain(self.y + y):
          fraggle.x = self.x + x
          fraggle.y = self.y + y
          if fraggle not in sandbox:
            sandbox.append(fraggle)
            possibles.append(fraggle._path_to(dest, depth-1,
                                              sandbox, debug=debug))
    distance = 999
    best = []
    for path in possibles:
      if path and len(path) < distance:
        best = path
        distance = len(path)
    if best:
      best.insert(0, Pixel(self.x, self.y))
    return best


  def path_to(self, dest: Pixel, depth: int,
                    sandbox: List[Pixel],
                    debug: bool = False) -> List[Pixel]:
    sandbox_copy = [ p for p in sandbox ]
    path = self._path_to(dest, depth, sandbox_copy, debug=debug)
    if debug: print(f"finally: {defs.listr(sandbox_copy)}")
    return path


  '''
  looking for something to do
  - if there are any plan spots without a brick, FETCHING
  - if there are any bricks not in the plan, switch to CLEANING
  - if I'm adjacent to anything, move to an open spot
  '''
  def rest(self, plans: List[Pixel], sandbox: List[Pixel]):
    if self.open(plans, sandbox):
      self.state = FETCHING
    elif self.find(BRICK_COLOR, self.subtract(self.open(sandbox, plans), 
                                              self.dumps[0]+self.dumps[1])):
      self.state = CLEANING
    elif self.adjacent(sandbox):
      # half-speed wander
      if random.random() < 0.50:
        self.wander(sandbox)
    # else:
    #   print(f"{self} resting")


  '''
  walk to a brick in the sandbox which is not in the plans,
  pick it up, set state to BUILDING
  '''
  def fetch(self, plans: List[Pixel], sandbox: List[Pixel],
            debug: bool = False):
    # need to rethink this
    # bail out: if I'm on my target ...
    if False and len(plans) == 1 and self == plans[0]:
      if debug: defs.debug(f"{self} fetching {plans[0]} ... ", end='')
      # ... and it's a brick, take it
      if plans[0].color == BRICK_COLOR:
        if debug: defs.debug("brick!")
        brick = plans[0]
        # FALLTHROUGH
      else:
        # not a brick?  do something else
        if debug: defs.debug("no brick, rest")
        self.state = RESTING
        return

    # if I'm next to a brick, pick it up & build
    bricks = self.find(BRICK_COLOR, self.open(sandbox, plans))
    brick = self.adjacent(bricks)

    if brick and not isinstance(brick, Fraggle):
      defs.debug(f"{self} removing {brick}")
      sandbox.remove(brick)
      self.state = BUILDING
    elif bricks:
      # otherwise, keep seeking that brick
      if debug: defs.debug(f"{self} seeking {defs.listr(bricks)}")
      self.seek(bricks, sandbox, wobble=0.10)
    else:
      # no bricks to fetch; rest
      if debug: defs.debug(f"{self} no bricks, resting")
      self.state = RESTING


  '''
  ideal: seek a pixel, and keep trying to go there 
         as long as it is a valid target and I don't get stuck

  seek() returns a target Pixel, or True (wandered), or False (blocked)
  '''
  def seek(self, plans, sandbox, wobble, debug: bool = False):
    if self.state == self.last_state \
        and self.sought_target:
      if debug: defs.debug(f"{self} same old seek")
      self.sought_target = super().seek([self.sought_target], sandbox, wobble)
      # path = self.path_to(self.sought_target, 6, sandbox)
      # if path:
      #   super().seek(path, sandbox, wobble)
      # else:
      #   self.sought_target = super().seek([self.sought_target], sandbox, wobble)
    else:
      self.last_state = self.state
      if debug: defs.debug(f"{self} intercepted a seek")
      self.sought_target = super().seek(plans, sandbox, wobble)
    return self.sought_target


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
        self.seek(self.open(plans, sandbox), sandbox, wobble=0.10)
      else:
        self.state = RESTING


  '''
  walk to a loose brick, pick it up, dump it
  almost identical to fetch()

  TODO: don't clean stuff on the far right
  '''
  def clean(self, plans: List[Pixel], sandbox: List[Pixel], 
                  debug: bool = False):
    # if I'm next to a brick, pick it up & dump -- same as fetch
    bricks = self.find(BRICK_COLOR, self.open(sandbox, plans))
    if debug: defs.debug(f"{self}: cleaning {defs.listr(bricks)}")
    # include only x < defs.SIDE-2  (TODO: not in dumps)
    # bricks = [ b for b in bricks if b.x < defs.SIDE-2 ]
    bricks = self.open(bricks, self.dumps[0] + self.dumps[1])
    if debug: defs.debug(f"{self}: now cleaning {defs.listr(bricks)}")
    if bricks:
      brick = self.adjacent(bricks)
      if brick and not isinstance(brick, Fraggle):
        defs.debug(f"{self} removing {brick}")
        sandbox.remove(brick)
        self.state = DUMPING
      else:
        # otherwise, keep seeking that brick
        self.seek(bricks, sandbox, wobble=0.10)
    else:
      self.state = RESTING


  '''
  almost identical to build()
  '''
  def dump(self, plans: List[Pixel], sandbox: List[Pixel],
                 debug: bool = False):
    dumps = [ [Pixel(x, defs.SIDE-2) for x in range(defs.SIDE)],
              [Pixel(x, defs.SIDE-3) for x in range(defs.SIDE)] ]

    # TODO: if any free in dumps[i], put it there
    dump = self.open(self.dumps[0], sandbox) \
             or self.open(self.dumps[1], sandbox)
    if debug: defs.debug(f"{self} dumping in {defs.listr(dump)}")

    corner = Pixel(defs.SIDE-1, defs.SIDE-1)
    dump = corner.nearest(self.open(dumps[0], sandbox) \
                          or self.open(dumps[1], sandbox))[0]

    if debug: defs.debug(f"{self} dumping in {dump}")
    
    if self.adjacent([dump]):
      spot = self.adjacent([dump])
      if debug: defs.debug(f"{self}: dumping in {spot}")
      brick = Pixel(spot.x, spot.y, BRICK_COLOR)
      sandbox.append(brick)
      self.state = RESTING
    else:
      # walk to the dump
      self.seek([dump], sandbox, wobble=0.10)

  
  def am_i_stuck(self):
    if self.last_loc == self:
      self.stuckness += 1
      if self.stuckness >= STUCK_COUNTER:
        self.stuckness = 0
        self.pre_stuck_state = self.state
        self.state = STUCK
        self.sticky_start = Pixel(self.x, self.y)
    else:
      self.stuckness = 0
      self.last_loc = Pixel(self.x, self.y)


  '''
  I think I got stuck!  wander around for a while
  '''
  def unstick(self, plans: List[Pixel], sandbox: List[Pixel]):
    if self.distance_to(self.sticky_start) >= UNSTUCK_DIST:
      self.state = self.pre_stuck_state
    else:
      self.wander(sandbox)


  # manages states, but NOT state transitions
  def run(self, plans: List[Pixel], sandbox: List[Pixel],
                debug: bool = False):
    if self.state == STUCK:
      self.unstick(plans, sandbox)
    elif self.state == FETCHING:
      self.fetch(plans, sandbox, debug=debug)
      self.am_i_stuck()
    elif self.state == BUILDING:
      self.build(plans, sandbox)
      self.am_i_stuck()
    elif self.state == CLEANING:
      self.clean(plans, sandbox, debug)
      self.am_i_stuck()
    elif self.state == DUMPING:
      self.dump(plans, sandbox, debug)
      self.am_i_stuck()
    else:
      self.state = RESTING # just in case
      self.rest(plans, sandbox)
    self.color = colors[self.state]
