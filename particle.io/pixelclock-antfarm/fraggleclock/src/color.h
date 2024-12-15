#ifndef COLOR_H
#define COLOR_H

#include <neopixel.h>
#include "defs.h"

typedef uint32_t color_t;

#define RED         (Adafruit_NeoPixel::Color(255, 0, 0))
#define MIDRED      (Adafruit_NeoPixel::Color(128, 0, 0))
#define DARKRED     (Adafruit_NeoPixel::Color(64, 0, 0))
#define GREEN       (Adafruit_NeoPixel::Color(0, 255, 0))
#define MIDGREEN    (Adafruit_NeoPixel::Color(0, 128, 0))
#define DARKGREEN   (Adafruit_NeoPixel::Color(0, 64, 0))
#define BLUE        (Adafruit_NeoPixel::Color(0, 0, 255))
#define LIGHTBLUE   (Adafruit_NeoPixel::Color(0, 0, 64))
#define YELLOW      (Adafruit_NeoPixel::Color(255, 255, 0))
#define BLACK       (Adafruit_NeoPixel::Color(0, 0, 0))
#define DARKGREY    (Adafruit_NeoPixel::Color(8, 8, 8))
#define LIGHTGREY   (Adafruit_NeoPixel::Color(32, 32, 32))
#define DARKWHITE   (Adafruit_NeoPixel::Color(64, 64, 64))
#define MIDWHITE    (Adafruit_NeoPixel::Color(128, 128, 128))
#define WHITE       (Adafruit_NeoPixel::Color(255, 255, 255))
#define YELLOWGREEN (Adafruit_NeoPixel::Color(64, 255, 0))
#define CYAN        (Adafruit_NeoPixel::Color(0, 255, 255))
#define MAGENTA     (Adafruit_NeoPixel::Color(255, 0, 255))

color_t main_color;


void store_color() {
  EEPROM.put(COLOR_ADDY, main_color);
} // store_color()


void load_color() {
  EEPROM.get(COLOR_ADDY, main_color);
} // load_color()


// data should contain 3 ints, like 128 0 0 -> reddish
int change_color(String data) {
  int i = data.indexOf(" ");
  if (i == 0) {
    return -1; // error in the first position
  }
  int r = data.toInt();
  data = data.substring(i+1);
  int g = data.toInt();
  i = data.indexOf(" ");
  if (i == 0) {
    return -2; // error in the second position
  }
  data = data.substring(i);
  int b = data.toInt();
  main_color = Adafruit_NeoPixel::Color(r, g, b);
  store_color();
  return (r+1)/32*100 + (g+1)/32*10 + (b+1)/32;
} // int change_color(data)


void setup_color() {
  Particle.function("change_color", change_color);
  load_color();
} // setup_color()

#endif
