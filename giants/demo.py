#!/usr/bin/env python3 

import board
import neopixel
import time

pixels = neopixel.NeoPixel(board.D18, 256)
for i in range(256):
  pixels[i] = (0,0,32)
  time.sleep(0.25)
  pixels[i] = (0,0,0)
time.sleep(5)
pixels.fill((0, 0, 0))
