# An rpi-based pixelclock, with "neopixel" WS2812-based 16x16 grid

## Hardware:

* rpi zero W
* amazon 16x16 RGB grid
* solder directly to D18, +5V, GND

## Software: 

* adafruit's circuitpython stuff
   * `sudo pip3 install adafruit-circuitpython-bh1750`
* this

## Theory of Operation

* it uses "wobbly time", which means "it will be a little fast".  Read the 
code.
* The time is displated H H / M M.  Invisible "food" is sprinkled in the 
shape of numbers, ants are released (or culled) based on the amount of food.
Ants will seek open food based on a pretty simple set of rules:
   * if I have food, 95% of the time do nothing 
   * if I'm touching food, move directly to it
   * if there's any open food, walk toward it
   * otherwise, roam around
* ants can't step on ants, but between culling and 5% chance wandering,
they tend to mostly form the shape of numbers within a few seconds

## RPI setup / installation

1 copy fraggleclock.service to /etc/systemd/system
2 `sudo systemctl daemon-reload`

### de-install
1 `sudo rm /etc/systemd/system/fraggleclock.service`
2 `sudo systemctl daemon-reload`

The service should now be running, clock should be live.

### manual start / stop
* `sudo systemctl start antclock.service`
* `sudo systemctl stop antclock.service`

