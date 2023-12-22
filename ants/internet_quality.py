from typing import List
from time import sleep
from datetime import datetime, timedelta
import random

from pixel import Pixel, Ant
from timer import Timer
import ping
from ping import ping_forever
from mproc import ForeverProcess
from defs import map_basic, SIDE


# displays a graph showing internet quality
# can be run FREQUENTLY without hosing everything up
class InternetQuality: 
  def __init__(self, sandbox: List[Pixel], interval: int):
    self.sandbox = sandbox
    self.timer = Timer(timedelta(seconds=interval))
    self.pinger = ForeverProcess(ping_forever)
    self.age = 999
    self.rtt = 999
    self.graph = []
    self.pinger.start()
    self._read_ping_data()


  def __del__(self):
    self.pinger.stop()

  # returns 1..4 stars
  def _rtt_score(self):
    return 5 - int(map_basic(self.rtt, 1, 500, 1, 4))


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
        if ant.x == 0:
          self.sandbox.remove(ant)
          self.graph.remove(ant)
        else:
          ant.step(-1, 0, self.sandbox)
      # if (SIDE-1, SIDE-1) is free...
      new_ant = Ant(SIDE-1, SIDE-1, c[self._rtt_score()])
      print(f"trying {new_ant}")
      if new_ant not in self.sandbox:
        self.sandbox.append(new_ant)
        self.graph.append(new_ant)
      print(f"Graph: {len(self.graph)} ants")

