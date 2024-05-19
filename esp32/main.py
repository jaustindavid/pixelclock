import random
import defs
from font import get_time
from pixel import Pixel
from fraggle import Fraggle
import fraggle as f
from matrix import Matrix
from timer import Timer


# rolls the dice
def p(f: float) -> float:
  return random.random() < f


'''
if there are not enough bricks, try to add one in a corner
'''
def manage_bricks(plans: list[Pixel], sandbox: list[Pixel]):
  fraggle = Fraggle()
  bricks = fraggle.find(f.BRICK_COLOR,sandbox)
  print(f"# bricks: {len(bricks)}")
  print(f"plan calls for: {len(plans)}")
  if len(bricks) < len(plans):
    new_brick = Pixel(defs.SIDE-1, defs.SIDE-2, f.BRICK_COLOR)
    if new_brick not in sandbox:
      sandbox.append(new_brick)

'''
TODO: port this
graph_dots = []
iq_timer = Timer(15)
def graph_iq(iq: internet_quality.InternetQuality, sandbox: list[Pixel]):
  if iq_timer.expired():
    if graph_dots and graph_dots[0].x == 0:
      sandbox.remove(graph_dots[0])
      graph_dots.remove(graph_dots[0])
    for dot in graph_dots:
      dot.step(-1,0, sandbox)
    score = min(iq.age_score(), iq.rtt_score())
    print(f"IQ: {score}, age={iq.age}, rtt={iq.rtt}")
    c = { 4: 'G', 3: 'L', 2: 'Y', 1: 'R' }
    dot = Pixel(defs.SIDE-1, defs.SIDE-1, c[score])
    if dot not in graph_dots:
      graph_dots.append(dot)
      sandbox.append(dot)
'''



if __name__ == "__main__":
  fraggles = [Fraggle() for i in range(2)]
  sandbox = [ Pixel(x, defs.SIDE-2, f.BRICK_COLOR) 
              for x in range(defs.SIDE) ] \
          + [ Pixel(x, defs.SIDE-3, f.BRICK_COLOR) 
              for x in range(defs.SIDE) ]
  sandbox.extend(fraggles)
  plans = get_time(f.BRICK_COLOR)
  matrix = Matrix(defs.SIDE, sandbox)
  # TODO iq = internet_quality.InternetQuality(15)
  loop = Timer(0.25) # ratelimit: 4 fps
  second = Timer(1)
  frames = 0
  while True:
    if second.expired():
      manage_bricks(plans, sandbox)
      print(f"fraggles: {defs.listr(fraggles)}")
      print(f"plans ({len(plans)}): {defs.listr(plans)}")
      print(f"sandbox ({len(sandbox)}): {defs.listr(sandbox)}")
      print(f"{frames} frames per second")
      print(str(matrix))
      plans = get_time()
      frames = 0
      # TODO iq.run()
    # TODO graph_iq(iq, sandbox)
    for fraggle in fraggles:
      fraggle.run(plans, sandbox)
    matrix.show()
    frames += 1
    loop.wait()
  print(str(matrix))

iq = None
