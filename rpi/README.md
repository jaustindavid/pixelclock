# An rpi-based pixelclock, with "neopixel" WS2812 16x16 grid

## Hardware:

* rpi zero W
* amazon 16x16 RGB grid
   * solder directly to D18, +5V, GND
* adafruit BH1750 & qwiic cable
   * pin or solder to +3.3V, SDA, SCL, GND
* laser up a nice enclosure

## Software: 

* adafruit's circuitpython stuff
   * `sudo pip3 install adafruit-circuitpython-bh1750`
* this

## Theory of Operation

* it uses "wobbly time", which means "it will be a little fast".  Read the 
code.
* The time is displated H H / M M.  

### Antfarm
Invisible "food" is sprinkled in the shape of numbers (HH/MM).  A queen roams
around and either makes or eats ants based on the availability of food.

Ants will seek open food based on a pretty simple set of rules:
   * if I have food, 95% of the time do nothing 
   * if I'm touching food, move directly to it
   * if there's any open food, walk toward it
   * otherwise, roam around
* ants can't step on ants, but between culling and 5% chance wandering,
they tend to mostly form the shape of numbers within a few seconds

### Fraggles

Invisible "plans" are written in the shape of numbers (HH/MM).
Two fraggles (nb yeah I know doozers build) grab bricks off the bottom,
or any stray bricks on screen, and fill in the plan.

If the plan is complete, any trash bricks are moved back to the bottom

## Network Latency Graph

A simple graph will show along the bottom, rolling from right (newest) to
left (oldest data).  Every 15 seconds it pings google.com and writes a
green dot (<50ms), yellowish, yellow, or red (>= 500ms) based on latency.

It's a timeline read from left -> right, history up to present.

## RPI setup / installation

1 edit pixelclock.service, then copy it to /etc/systemd/system
2 `sudo systemctl daemon-reload`
3 `sudo systemctl enable pixelclock.service`
4 link (ln -s ...) pixelclock.py to the flavor you prefer

### de-install

1 `sudo rm /etc/systemd/system/pixelclock.service`
2 `sudo systemctl daemon-reload`

The service should now be running, pixelclock should be live.

### manual start / stop
* `sudo systemctl start pixelclock.service`
* `sudo systemctl stop pixelclock.service`

