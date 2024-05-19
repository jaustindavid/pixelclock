import random
import time

# micropython doesn't have datetime :/

class WobblyTime:
    def __init__(self, min_fastness, max_fastness):
        self.min_fastness = min_fastness
        self.max_fastness = max_fastness
        self.last_tick = time.time()
        self.target = 0
        self.advance = 0
        self.vtime = time.time()
        self.recalculate()


    def recalculate(self):
        now = time.time()
        elapsed_seconds = (now - self.last_tick)
        # print(f"elapsed: {elapsed_seconds}")
        # print(f"advance: {self.advance}, target: {self.target}")
        if self.advance == self.target:
          self.target = random.randint(self.min_fastness, self.max_fastness)
        elif self.advance < self.target:
          # print(f"adv < target")
          self.advance = min(self.target, self.advance + 2*elapsed_seconds)
        else:
          # print(f"adv > target")
          self.advance = max(self.target, self.advance - elapsed_seconds/2)
        self.last_tick = now


    def gettime(self):
        now = time.time()
        self.recalculate()
        self.vtime = now + self.advance
        hh, mm, ss = time.localtime(self.vtime)[3:6]
        return hh, mm, ss


if __name__ == "__main__":
  # Example usage
  wobbly_time = WobblyTime(10, 30)
  while True:
    hours, minutes, seconds = wobbly_time.gettime()
    (hh, mm, ss) = time.localtime()[3:6]
    print(f"Wobbly time: {hours:02}:{minutes:02}:{seconds:02}")
    print(f"Actual time: {hh:02}:{mm:02}:{ss:02}")
    time.sleep(5)

