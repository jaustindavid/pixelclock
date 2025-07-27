#!/bin/env python

import json
import time
import urllib.request
import urllib.parse
import wx_api_key


class OpenWeather:
    def __init__(self, city='Charleston', LAT=32.8502493, LON=-80.0635325):
        self.city = city
        self.LAT = LAT
        self.LON = LON
        self.last_refresh = None
        self.URL = self.build_url()
        self.latest_weather = None

    def build_url(self):
        API_KEY=wx_api_key.OPEN_WEATHER_TOKEN
        return f'https://api.openweathermap.org/data/3.0/onecall?lat={self.LAT}&lon={self.LON}&exclude=minutely,daily,alerts&appid={API_KEY}'

    def update(self, interval=600):
        """pulls the decoded JSON from OW, if interval has expired"""
        if (not self.last_refresh) \
            or (time.monotonic() - self.last_refresh) > interval:
    
            response = urllib.request.urlopen(self.URL)
            if response.getcode() == 200:
                value = response.read()
                self.last_refresh = time.monotonic()
                self.latest_weather = json.loads(value.decode("utf-8"))

    def city(self):
        return self.city

    def forecast(self, offset_hours=None):
        self.update()
        if offset_hours is None:
            return self.latest_weather['current']
        else:
            return self.latest_weather['hourly'][offset_hours]
    
    def feels_like(self):
        self.update()
        return self.c(self.latest_weather['current']['feels_like'])

    def range(self, start, end):
        self.update()
        temps = [ hour['temp'] for hour in self.latest_weather['hourly'][start:end+1] ]
        return (self.c(min(temps)), self.c(max(temps)))

    def pop(self, threshold, start, end):
        """returns first (index, pop) over threshold in the range"""
        self.update()
        for i in range(start, end+1):
            hour = self.latest_weather['hourly'][i]
            if hour['pop'] >= threshold:
                return (i, hour['pop'])
        return None
        

    def c(self, k):
        return k - 273.15

    def f(self, c):
        return c * 9/5 + 32


if __name__ == "__main__":
  ow = OpenWeather()
  print(ow.city)
  print(ow.forecast()['weather'][0]['icon'])
  print(f'Feels like {ow.feels_like():5.2f}C = {ow.f(ow.feels_like()):5.2f}F')
  print(ow.range(0, 11))
  print(ow.pop(0.25, 0, 11))
