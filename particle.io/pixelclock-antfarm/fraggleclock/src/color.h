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


color_t palette[4] = 
  {
     MIDGREEN,                             // 0: time (active)
     MIDWHITE,                             // 1: sprite
     DARKRED,                              // 2: alt (dirty)
     MAGENTA                               // 3: undef
  };

// palette indices
#define TIME_COLOR   (palette[0])
#define SPRITE_COLOR (palette[1])
#define ALT_COLOR    (palette[2])


struct color_struct {
  int8_t version;
  color_t pal[4];
};


void store_colors() {
  struct color_struct datum;
  datum.version = 3;
  memcpy(datum.pal, palette, sizeof(palette));
  EEPROM.put(COLOR_ADDY, datum);
} // store_colors()


void load_colors() {
  struct color_struct datum;
  EEPROM.get(COLOR_ADDY, datum);
  if (datum.version == 3) {
    memcpy(palette, datum.pal, sizeof(palette));
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


int rgbify(color_t c) {
  byte r = (c&0xFF0000) >> 16;
  byte g = (c&0x00FF00) >> 8;
  byte b = (c&0x0000FF);
  return (r+1)/32*100 + (g+1)/32*10 + (b+1)/32;
}


// data should contain 3 ints, like 128 0 0 -> reddish
int change_palette(String data) {
  int index = data.toInt();
  int i = data.indexOf(" ");
  if (i == 0) {
    return -1; // error in the first position
  }
  data = data.substring(i+1);

  color_t new_color = parse_color(data);

  if (new_color == -1) {
   return -2;
  }
 
  if (new_color == -2) {
   return -3;
  }

  palette[index] = new_color;
  store_colors();

  return index * 1000 + rgbify(new_color);
} // int change_palette(data)



void setup_color() {
  Particle.function("change_palette", change_palette);
  load_colors();
} // setup_color()

#endif
