from machine import Pin
from neopixel import NeoPixel

pin = Pin(13, Pin.OUT)   # set GPIO0 to output to drive NeoPixels
np = NeoPixel(pin, 256)   # create NeoPixel driver on GPIO0 for 8 pixels
np[0] = (255, 255, 255) # set the first pixel to white
np.write()              # write data to all pixels
r, g, b = np[0]         # get first pixel colour

while True:
  for i in range(64):
    r, g, b = np[i]         # get first pixel colour
    np[i] = (r+4)%256,(g+4)%256,(b+4)%256
  np.write()
