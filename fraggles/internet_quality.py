from typing import List
from time import sleep
from datetime import datetime
import random

from pixel import Pixel, Ant
from timer import Timer
import ping
from ping import ping_forever
from mproc import ForeverProcess
from defs import map_basic, SIDE, listr


# displays a graph showing internet quality
# can be run FREQUENTLY without hosing everything up
class InternetQuality: 
  def __init__(self, interval: int):
    self.timer = Timer(interval)
    self.pinger = ForeverProcess(ping_forever)
    self.age = 999
    self.rtt = 999
    self.pinger.start()
    self._read_ping_data()


  def __del__(self):
    self.pinger.stop()


  # returns 1..4 stars
  def rtt_score(self):
    return 5 - int(map_basic(self.rtt, 1, 500, 1, 4))


  def age_score(self):
    return 5 - int(map_basic(self.age, 15, 60, 1, 4))


  def _read_ping_data(self):
    (rtt, latency_score, latency_timestamp) = ping.get_score()
    self.age = datetime.now().timestamp() - latency_timestamp
    self.rtt = rtt
    print(f"ping data: score={latency_score}, rtt={rtt}, age: {self.age}")


  def run(self): 
    if self.timer.expired():
      self._read_ping_data()
