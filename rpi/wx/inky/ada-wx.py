#!/bin/env python

import adafruit_display_wrapper
import time
import wobblytime
import open_weather
import weather_gfx


adw = adafruit_display_wrapper.AdafruitDisplayWrapper()
display = adw.display

wt = wobblytime.WobblyTime()
ow = open_weather.OpenWeather()
gfx = weather_gfx.WeatherGraphics(adafruit_display=adw.display)


last_hhmm = "0000"
while (True):
    hhmm = f"{wt.hour():02d}:{wt.minute():02d}"
    if hhmm != last_hhmm:
        last_hhmm = hhmm
        gfx.update(ow, hh=wt.hour(), mm=wt.minute())
        print(f"{hhmm}: {wt}")
    time.sleep(1)
