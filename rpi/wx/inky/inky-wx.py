#!/usr/bin/env python3

import argparse
import open_weather
import wobblytime
import weather_gfx

from font_hanken_grotesk import HankenGroteskBold, HankenGroteskMedium
from font_intuitive import Intuitive
from PIL import Image, ImageDraw, ImageFont

from inky.auto import auto

def getsize(font, text):
    _, _, right, bottom = font.getbbox(text)
    return (right, bottom)

try:
    inky_display = auto(ask_user=True, verbose=True)
except TypeError:
    raise TypeError("You need to update the Inky library to >= v1.1.0")

try:
    inky_display.set_border(inky_display.RED)
except NotImplementedError:
    pass

print(inky_display.resolution)
# 250 x 122


wt = wobblytime.WobblyTime()
ow = open_weather.OpenWeather()
gfx = weather_gfx.WeatherGraphics(inky_display=inky_display)

last_hhmm = "0000"
while True:
    hhmm = f"{wt.hour():02d}:{wt.minute():02d}"
    if hhmm != last_hhmm:
        last_hhmm = hhmm
        gfx.update(ow,
                   hh=wt.hour(), 
                   mm=wt.minute())
        print(f'{hhmm}: {wt}')
