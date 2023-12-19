import random
from datetime import datetime, timedelta
import time


class WobblyTime:
    def __init__(self, min_fastness, max_fastness):
        self.min_fastness = min_fastness
        self.max_fastness = max_fastness
        self.last_tick = datetime.now()
        self.target = 0
        self.advance = 0
        self.vtime = datetime.now()
        self.recalculate()


    def recalculate(self):
        now = datetime.now()
        elapsed_seconds = (now - self.last_tick).total_seconds()
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
        now = datetime.now()
        self.recalculate()
        self.vtime = now + timedelta(seconds=self.advance)
        return self.vtime.hour, self.vtime.minute, self.vtime.second
        # Update the fastness multiplier with a random value within the bounds
        self.fastness_multiplier = random.uniform(self.min_fastness, self.max_fastness)

        # Ensure the fastness_multiplier doesn't make time go backward
        if self.fastness_multiplier < 0 and self.last_time + timedelta(seconds=self.min_fastness) > datetime.now():
            self.fastness_multiplier = -self.fastness_multiplier

        # Calculate the wobbly time based on the actual time and fastness multiplier
        wobbly_time = self.last_time + timedelta(seconds=random.random() * self.fastness_multiplier)

        # Ensure the wobbly time is at least min_fastness ahead of actual time
        wobbly_time = max(wobbly_time, self.last_time + timedelta(seconds=self.min_fastness))

        # Update the last time for the next call
        self.last_time = wobbly_time

        # Return the wobbly time as a tuple of hours, minutes, seconds
        return wobbly_time.hour, wobbly_time.minute, wobbly_time.second

if __name__ == "__main__":
  # Example usage
  wobbly_time = WobblyTime(10, 30)
  while True:
    hours, minutes, seconds = wobbly_time.gettime()
    now = datetime.now()
    print(f"Wobbly time: {hours:02}:{minutes:02}:{seconds:02}")
    print(f"Actual time: {now.hour:02}:{now.minute:02}:{now.second:02}")
    time.sleep(5)

