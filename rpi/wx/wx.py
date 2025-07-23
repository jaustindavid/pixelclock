#!/bin/env python
# /home/austin/adafruit-epd/venv/bin/python

import urllib.request
import urllib.parse
import board
import busio
import digitalio
from adafruit_epd.ssd1680 import Adafruit_SSD1680Z
from PIL import Image, ImageDraw, ImageFont
import time
import wobblytime
import digitalio
from weather_graphics import Weather_Graphics
import wx_api_key

up_button = digitalio.DigitalInOut(board.D5)
up_button.switch_to_input()
down_button = digitalio.DigitalInOut(board.D6)
down_button.switch_to_input()
MODE_NORMAL = 0

# RGB Colors
WHITE = (255, 255, 255)
BLACK = (0, 0, 0)


def clear(display):
    image = Image.new("RGB", (display.width, display.height))
    draw = ImageDraw.Draw(image)

    # Draw a filled box as the background
    draw.rectangle((0, 0, display.width - 1, display.height - 1), fill=BLACK)

    # Draw a smaller inner foreground rectangle
    BORDER = 4
    draw.rectangle(
       (BORDER, BORDER, display.width - BORDER - 1, display.height - BORDER - 1),
           fill=WHITE,
           )
    display.image(image)
    display.display()


def render(display, render_font, text):
    image = Image.new("RGB", (display.width, display.height))
    draw = ImageDraw.Draw(image)

    # Draw a smaller inner foreground rectangle
    BORDER = 4
    draw.rectangle(
       (BORDER, BORDER, display.width - BORDER - 1, display.height - BORDER - 1),
           fill=WHITE,
           )
    (font_width, font_height), (offset_x, offset_y) = render_font.font.getsize(text)
    draw.text(
        (display.width // 2 - font_width // 2, display.height // 2 - font_height // 2),
        text,
        font=render_font,
        fill=BLACK,
)
    display.image(image)
    print(text)
    display.display()


weather_refresh = None
def wx_update():
    global weather_refresh
    if (not weather_refresh) or (time.monotonic() - weather_refresh) > 600:
        response = urllib.request.urlopen(data_source)
        if response.getcode() == 200:
            value = response.read()
            print("Response is", value)
            weather_refresh = time.monotonic()
            return value
    return None


# Use cityname, country code where countrycode is ISO3166 format.
# E.g. "New York, US" or "London, GB"
LOCATION = "Charleston, US"
DATA_SOURCE_URL = "http://api.openweathermap.org/data/2.5/weather"

if len(wx_api_key.OPEN_WEATHER_TOKEN) == 0:
        raise RuntimeError(
                "You need to set your token first. If you don't already have one, you can register for a free account at https://home.openweathermap.org/users/sign_up"
                    )

# Set up where we'll be fetching data from
params = {"q": LOCATION, "appid": wx_api_key.OPEN_WEATHER_TOKEN}
data_source = DATA_SOURCE_URL + "?" + urllib.parse.urlencode(params)

# Define SPI and pins
spi = busio.SPI(board.SCK, MOSI=board.MOSI, MISO=board.MISO)
ecs = digitalio.DigitalInOut(board.CE0)  # Chip select
dc = digitalio.DigitalInOut(board.D22)   # Data/command
rst = digitalio.DigitalInOut(board.D27)  # Reset
busy = digitalio.DigitalInOut(board.D17) # Busy

# Initialize the display with SSD1680Z driver
display = Adafruit_SSD1680Z(
            122, 250, spi, cs_pin=ecs, dc_pin=dc, sramcs_pin=None, rst_pin=rst, busy_pin=busy
                        )

# Set display to portrait mode
display.rotation = 3

# Load a TTF Font
banner_font = ImageFont.truetype(
    "/usr/local/share/fonts/custom/Hack-Regular.ttf", 32
)
time_font = ImageFont.truetype(
    "/usr/local/share/fonts/custom/Hack-Regular.ttf", 64
)

clear(display)
render(display, banner_font, "Wobbly Time")
time.sleep(5)

gfx = Weather_Graphics(display, am_pm=True, celsius=False)
weather_refresh = None

wt = wobblytime.WobblyTime()
last_hhmm = "0000"
while (True):
    hhmm = f"{wt.hour():02d}:{wt.minute():02d}"
    if hhmm != last_hhmm:
        last_hhmm = hhmm
        value = wx_update()
        hh = f"{wt.hour():02d}"
        mm = f"{wt.minute():02d}"
        if value:
            gfx.display_weather(value, hh, mm)
        else:
            gfx.update_time(hh, mm)
        print(hhmm)
    time.sleep(1)
