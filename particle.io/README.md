A Particle Photon implementation of the ant clock.

## Performance

I tested, Adafruit Neopixel is faster than FastLED for simple ant-animation:
single pixel motion and redraw.  

The whole thing runs at ~ 1-2ms per screen draw with 50-75 active pixels.

Motion is blocky but could conceivably run at 60fps without trouble, 
apart from maybe some light epilepsy.
