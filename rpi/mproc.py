"""write me a simple class in python that runs a function in a separate process forever (while True: function)"""

import multiprocessing
import time


class ForeverProcess:
    def __init__(self, target_function):
        self.target_function = target_function
        self.process = None

    def start(self):
        self.process = multiprocessing.Process(target=self.run)
        self.process.start()

    def run(self):
        while True:
            self.target_function()

    def stop(self):
        if self.process:
            self.process.terminate()
            self.process.join()


def my_function():
    print("Running my function in a separate process!")
    time.sleep(1)  # Simulate some work

if __name__ == "__main__":
  # Example usage

  process = ForeverProcess(my_function)
  print("starting")
  process.start()

  # Do other things while the function runs in the background
  time.sleep(5)

  print("stopping")
  process.stop()

  print("Forever process stopped.")

