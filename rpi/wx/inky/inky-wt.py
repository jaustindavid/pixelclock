#!/bin/env python
# /home/austin/adafruit-epd/venv/bin/python

from PIL import Image, ImageDraw, ImageFont
import time
import wobblytime
from inky.auto import auto

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
    display.set_image(image)
    display.show()


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
    display.set_image(image)
    print(text)
    display.show()


display = auto(ask_user=True, verbose=True)

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

wt = wobblytime.WobblyTime()
last_hhmm = "0000"
while (True):
    hhmm = f"{wt.hour():02d}:{wt.minute():02d}"
    if hhmm != last_hhmm:
        render(display, time_font, hhmm)
        last_hhmm = hhmm
    time.sleep(1)
