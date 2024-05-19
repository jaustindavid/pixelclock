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

#endif
