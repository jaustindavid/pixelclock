from typing import Union
from datetime import datetime, timedelta
from time import sleep

'''
A simple Timer class
  t = new Timer(10)
  t.wait() # returns in 10 seconds
  while True:
     if t.expired():
       # runs exactly once every 10 seconds
'''
class Timer:
  def __init__(self, interval: Union[timedelta, int]):
    if isinstance(interval, timedelta):
      self.interval = interval
    else:
      self.interval = timedelta(seconds=interval)
    self.start_time = datetime.now()


  # is this timer expired?  can be tested frequently,
  # returns True not more than once per interval
  def expired(self):
    if datetime.now() > self.start_time + self.interval:
      self.start_time = datetime.now()
      return True
    return False


  # wait until the timer would expire
  # useful to synchronize a loop
  def wait(self):
    if not self.expired():
      remaining = (self.start_time + self.interval - datetime.now()).total_seconds()
      if remaining > 0:
        sleep(remaining)
      self.start_time = datetime.now()


  # opposite of wait(): next call to expired() will return True
  def expire(self):
    self.start_time = datetime.now() - self.interval


'''
a stopwatch: start(), stop(), and read()

'''
class Stopwatch:
  def __init__(self):
    self.start()

  def start(self):
    self.start_time = datetime.now()
    self.elapsed = 0


  def stop(self):
    self.elapsed = (datetime.now() - self.start_time).total_seconds()


  def read(self):
    if self.elapsed:
      return self.elapsed
    else:
      return (datetime.now() - self.start_time).total_seconds()
