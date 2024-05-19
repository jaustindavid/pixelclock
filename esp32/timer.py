import time

'''
A simple Timer class
  t = new Timer(10)
  t.wait() # returns in 10 seconds
  while True:
     if t.expired():
       # runs exactly once every 10 seconds

  stripping datetime for the sake of micropython
'''
class Timer:
  def __init__(self, interval: int):
    self.interval = interval
    self.start_time = time.time()


  # is this timer expired?  can be tested frequently,
  # returns True not more than once per interval
  def expired(self):
    if time.time() > self.start_time + self.interval:
      self.start_time = time.time()
      return True
    return False


  # wait until the timer would expire
  # useful to synchronize a loop
  def wait(self):
    if not self.expired():
      remaining = self.start_time + self.interval - time.time()
      if remaining > 0:
        time.sleep(remaining)
      self.start_time = time.time()


  # opposite of wait(): next call to expired() will return True
  def expire(self):
    self.start_time = time.time() - self.interval - 1


'''
a stopwatch: start(), stop(), and read()

'''
class Stopwatch:
  def __init__(self):
    self.start()

  def start(self):
    self.start_time = time.time()
    self.elapsed = 0


  def stop(self):
    self.elapsed = time.time() - self.start_time


  def read(self):
    if self.elapsed:
      return self.elapsed
    else:
      return time.time() - self.start_time


if __name__ == "__main__":
    print("Timer demo... takes 3 seconds, should have no output")
    t = Timer(3)
    assert not t.expired()
    t.expire()
    time.sleep(0.1)
    assert t.expired()
    time.sleep(3.5)
    assert t.expired()

