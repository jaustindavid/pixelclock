# An rpi-based pixelclock, with "neopixel" WS2812-based 16x16 grid

## Hardware:

* rpi zero W
* amazon 16x16 RGB grid
   * solder directly to D18, +5V, GND
* adafruit BH1750 & qwiic cable
   * pin or solder to +3.3V, SDA, SCL, GND
* lazer up a nice enclosure

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

1 copy pixelclock.service to /etc/systemd/system
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

