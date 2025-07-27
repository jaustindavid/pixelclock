# SPDX-FileCopyrightText: 2020 Melissa LeBlanc-Williams for Adafruit Industries
#
# SPDX-License-Identifier: MIT

from datetime import datetime
import json
from PIL import Image, ImageDraw, ImageFont
from adafruit_epd.epd import Adafruit_EPD

small_font = ImageFont.truetype(
    "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", 16
)
medium_font = ImageFont.truetype("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 20)
large_font = ImageFont.truetype(
    "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", 24
)
time_font = ImageFont.truetype(
    "/usr/share/fonts/truetype/hack/Hack-Bold.ttf", 48
)
icon_font = ImageFont.truetype("./meteocons.ttf", 64)

# Map the OpenWeatherMap icon code to the appropriate font character
# See http://www.alessioatzeni.com/meteocons/ for icons
ICON_MAP = {
    "01d": "B",
    "01n": "C",
    "02d": "H",
    "02n": "I",
    "03d": "N",
    "03n": "N",
    "04d": "Y",
    "04n": "Y",
    "09d": "Q",
    "09n": "Q",
    "10d": "R",
    "10n": "R",
    "11d": "Z",
    "11n": "Z",
    "13d": "W",
    "13n": "W",
    "50d": "J",
    "50n": "K",
}

# RGB Colors
WHITE = (255, 255, 255)
BLACK = (0, 0, 0)

_X = 5
_CITY_Y = 4
# HH_X, HH_Y, MM_X
_HH_X = 20
_HH_Y = 22
_MM_X = 175
_MM_Y = _HH_Y
_MAIN_Y = 76
_DESCRIPTION_Y = 105
_TEMP_X = 175
_TEMP_Y = _MAIN_Y


class Weather_Graphics:
    def __init__(self, display, *, am_pm=True, celsius=True):
        self.am_pm = am_pm
        self.celsius = celsius

        self.small_font = small_font
        self.medium_font = medium_font
        self.large_font = large_font
        self.time_font = time_font

        self.display = display

        self._weather_icon = None
        self._city_name = None
        self._main_text = None
        self._feels_like = None
        self._temp_range = None
        self._description = None
        self._time_text = None
        self._time_hh = 0
        self._time_mm = 0

    def display_weather(self, weather, hh, mm):
        weather = json.loads(weather.decode("utf-8"))
        self.get_temps(weather)

        # set the icon/background
        self._weather_icon = ICON_MAP[weather["weather"][0]["icon"]]

        city_name = weather["name"] # + ", " + weather["sys"]["country"]
        print(city_name)
        self._city_name = city_name

        main = weather["weather"][0]["main"]
        print(main)
        self._main_text = main


        description = weather["weather"][0]["description"]
        description = description[0].upper() + description[1:]
        print(description)
        self._description = description
        # "thunderstorm with heavy drizzle"

        self.update_time(hh, mm)

    def update_time(self, hh, mm):
        self._time_hh = hh
        self._time_mm = mm
        self.update_display()

    def update_display(self):
        # clear -- big white rectangle
        self.display.fill(Adafruit_EPD.WHITE)
        image = Image.new("RGB", (self.display.width, self.display.height), color=WHITE)
        draw = ImageDraw.Draw(image)

        # Draw the Icon
        (font_width, font_height), (offset_x, offset_y) = icon_font.font.getsize(self._weather_icon)
        draw.text(
            (
                self.display.width // 2 - font_width // 2,
                self.display.height // 2 - font_height // 2 - 5,
            ),
            self._weather_icon,
            font=icon_font,
            fill=BLACK,
        )

        # Draw the city
        draw.text(
            (_X, _CITY_Y), self._city_name, font=self.small_font, fill=BLACK,
        )

        # Draw the time
        (font_width, font_height), (offset_x, offset_y) = time_font.font.getsize(self._time_hh)
        draw.text(
            (_HH_X, _HH_Y),
            self._time_hh,
            font=self.time_font,
            fill=BLACK,
        )
        draw.text(
            (_MM_X, _MM_Y),
            self._time_mm,
            font=self.time_font,
            fill=BLACK,
        )

        # Draw the main text
        (font_width, font_height), (offset_x, offset_y) = self.large_font.font.getsize(self._main_text)
        draw.text(
            (_X, _MAIN_Y),
            self._main_text,
            font=self.large_font,
            fill=BLACK,
        )
        # print(f'main: y={self.display.height - font_height * 2}')

        # Draw the description
        (font_width, font_height), (offset_x, offset_y) = self.small_font.font.getsize(self._description)
        draw.text(
            (5, _DESCRIPTION_Y),
            self._description,
            font=self.small_font,
            fill=BLACK,
        )
        # print(f'descr: y={self.display.height - font_height - 5}')

        # Draw the "feels like" temperature, bottom right
        (font_width, font_height), (offset_x, offset_y) = large_font.font.getsize(self._feels_like)
        X = self.display.width - _X - font_width
        draw.text(
            (X, _TEMP_Y), # (_TEMP_X, _TEMP_Y),
            self._feels_like,
            font=self.large_font,
            fill=BLACK,
        )

        # draw the range, top right
        (font_width, font_height), (offset_x, offset_y) = small_font.font.getsize(self._temp_range)
        X = self.display.width - _X - font_width
        draw.text(
            (X, _CITY_Y), self._temp_range, font=self.small_font, fill=BLACK,
        )


        self.display.image(image)
        self.display.display()

    def get_temps(self, weather):
        self._min_temp = weather["main"]["temp_min"] - 273.15
        self._max_temp = weather["main"]["temp_max"] - 273.15
        self._temp_range = f'{self._min_temp:2.1f}-{self._max_temp:2.1f}°C'
        print(f'range: {self._temp_range}')

        feels_like = weather["main"]["feels_like"] - 273.15  # its...in kelvin
        if self.celsius:
            self._feels_like = "%d °C" % feels_like
        else:
            self._feels_like = "%d °F" % ((feels_like * 9 / 5) + 32)
        print(f'feels like: {self._feels_like}')
