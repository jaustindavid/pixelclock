from datetime import datetime, timedelta
from time import sleep

class Timer:
  def __init__(self, interval: timedelta):
    self.interval = interval
    self.start = datetime.now()

  def expired(self):
    if datetime.now() > self.start + self.interval:
      self.start = datetime.now()
      return True
    return False

  def wait(self):
    if not self.expired():
      remaining = (self.start + self.interval - datetime.now()).total_seconds()
      sleep(remaining)
      self.start = datetime.now()
