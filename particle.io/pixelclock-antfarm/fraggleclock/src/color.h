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
#define LIGHTBLUE   (Adafruit_NeoPixel::Color(48, 48, 192))
#define ORANGE      (Adafruit_NeoPixel::Color(255, 64, 0))
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

color_t main_color   = MIDGREEN;
color_t sprite_color = MIDWHITE;

struct color_struct {
  byte version;
  color_t main;
  color_t sprite;
};


void store_colors() {
  struct color_struct datum;
  datum.version = 2;
  datum.main = main_color;
  datum.sprite = sprite_color;
  EEPROM.put(COLOR_ADDY, datum);
} // store_colors()


void load_color() {
  struct color_struct datum;
  EEPROM.get(COLOR_ADDY, datum);
  if (datum.version >= 2) {
    main_color = datum.main;
    sprite_color = datum.sprite;
  }
} // load_color()


// error values: 1 or 2
color_t parse_color(String data) {
  color_t parsed_color;
  int i = data.indexOf(" ");
  if (i == 0) {
    return 1; // error in the first position
  }
  int r = data.toInt();
  data = data.substring(i+1);
  int g = data.toInt();
  i = data.indexOf(" ");
  if (i == 0) {
    return 2; // error in the second position
  }
  data = data.substring(i);
  int b = data.toInt();
  parsed_color = Adafruit_NeoPixel::Color(r, g, b);
  return parsed_color;
} // color_t parse_color(data)


// data should contain 3 ints, like 128 0 0 -> reddish
int change_main_color(String data) {
  color_t new_color = parse_color(data);

  if (new_color == -1) {
   return -1;
  }
 
  if (new_color == -2) {
   return -2;
  }

  main_color = new_color;
  store_colors();

  byte r = (new_color&0xFF0000) >> 16;
  byte g = (new_color&0x00FF00) >> 8;
  byte b = (new_color&0x0000FF);
  return (r+1)/32*100 + (g+1)/32*10 + (b+1)/32;
} // int change_main_color(data)


// data should contain 3 ints, like 128 0 0 -> reddish
int change_sprite_color(String data) {
  color_t new_color = parse_color(data);

  if (new_color == -1) {
   return -1;
  }
 
  if (new_color == -2) {
   return -2;
  }

  sprite_color = new_color;
  store_colors();

  byte r = (new_color&0xFF0000) >> 16;
  byte g = (new_color&0x00FF00) >> 8;
  byte b = (new_color&0x0000FF);
  return (r+1)/32*100 + (g+1)/32*10 + (b+1)/32;
} // int change_main_color(data)

void setup_color() {
  Particle.function("change_main_color", change_main_color);
  Particle.function("change_sprite_color", change_sprite_color);
  load_color();
} // setup_color()

#endif
