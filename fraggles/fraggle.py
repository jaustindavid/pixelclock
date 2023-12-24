from pixels import Pixel

'''
A Fraggle is a Pixel which can retrieve a thing from a spot, 
then move it to another location.  It's a builder.  it has 
intent and state.

RESTING: looking for something to do; trying not to get in the way 
FETCHING: hunting for a new brick (dumping spot or not in plan)
BUILDING: taking the new brick to a free spot on the plan
CLEANING: taking a wild brick to a dumping spot

'''

# states
RESTING = 0
FETCHING = 1
BUILDING = 2
CLEANING = 3

class Fraggle(Pixel):
  def __init__(self):
    super().__init()
    self.state = RESTING


  '''
  looking for something to do
  - if I'm adjacent to anything, move to an open spot
  - if there are any bricks not in the plan, switch to CLEANING
  - if there are any plan spots without a brick, FETCHING
  '''
  def rest(self, plans: List[Pixel], sandbox: List[Pixel]):
    pass


  '''
  walk to a brick in the sandbox which is not in the plans,
  pick it up, set state to BUILDING
  '''
  def fetch(self, plans: List[Pixel], sandbox: List[Pixel]):
    pass


  '''
  walk to a spot in the plan, drop a brick, set state to RESTING
  '''
  def build(self, plans: List[Pixel], sandbox: List[Pixel]):
    pass


  def run(self):
    pass
