import time
import datetime
import random

class WobblyTime:
    def __init__(self, min_advance=120, max_advance=300):
        self.min_advance = min_advance
        self.max_advance = max_advance
        self._target_offset_seconds = random.randint(self.min_advance,
                                                     self.max_advance)
        self._offset_seconds = float(self._target_offset_seconds)
        self._last_tick_time = time.time()

    def __str__(self):
        self._tick()
        hh = datetime.datetime.now().hour
        mm = datetime.datetime.now().minute
        return (f"{hh}:{mm} -> "
                f"{self.hour()}:{self.minute()}; "
                f"offset {self._offset_seconds:5.2f}, "
                f"target {self._target_offset_seconds}")

    def _tick(self):
        time_elapsed = time.time() - self._last_tick_time
        if time_elapsed < 1:
            return
        self._last_tick_time = time.time()
        delta = self._target_offset_seconds - self._offset_seconds
        if abs(delta) <= time_elapsed:
            self._target_offset_seconds = random.randint(self.min_advance,
                                                         self.max_advance)
        elif self._offset_seconds < self._target_offset_seconds:
            self._offset_seconds += 0.3*time_elapsed
        else:
            self._offset_seconds -= 0.3*time_elapsed

    def minute(self):
        self._tick()
        current_datetime = datetime.datetime.now()
        adjusted_datetime = (current_datetime +
                             datetime.timedelta(seconds=self._offset_seconds))
        return adjusted_datetime.minute

    def hour(self):
        self._tick()
        current_datetime = datetime.datetime.now()
        adjusted_datetime = (current_datetime +
                             datetime.timedelta(seconds=self._offset_seconds))
        return adjusted_datetime.hour



if __name__ == "__main__":
  wt = WobblyTime()
  while (True):
    print(wt)
    time.sleep(3)
