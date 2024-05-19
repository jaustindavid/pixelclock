#!/usr/bin/env python3 

DEBUG=False
try:
  from machine import Pin
  DEBUG=False
except ModuleNotFoundError as e:
  pass

import random
import defs
from font import get_time
from pixel import Pixel
from matrix import Matrix
from timer import Timer
# TODO: import internet_quality 
from ant import Ant, Queen


if __name__ == "__main__":
  dots = [Pixel(8,defs.SIDE-1, 'g')]
  second = Timer(1)
  while True:
    dots[0].wander()
