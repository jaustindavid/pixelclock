/*
 * A class to manage a grid of LEDs
 * 
 * it renders dots.
 */
 
#ifndef DISPLAY_H
#define DISPLAY_H

#include <neopixel.h>
#include "defs.h"
#include "color.h"
#include "dot.h"
#include "list.h"

#undef PRINTF_DEBUGGER

class Display {
    private:
        color_t buf[MATRIX_X][MATRIX_Y];
        Adafruit_NeoPixel *neopixels;

        int txlate(Dot* dot) {
            int pixel = 0;
            
            pixel = dot->x * MATRIX_Y;
            if (dot->x%2 == 0) {
                pixel += dot->y % MATRIX_Y;
            } else {
                pixel += (MATRIX_Y - 1) - (dot->y % MATRIX_Y);
            }
            
            return pixel;    
        } // int Display::txlate(Dot)


    public:
        Display(Adafruit_NeoPixel *new_neopixels) {
            neopixels = new_neopixels;
            memset(buf, 0, sizeof(buf));
        }


        void init(void) {
            neopixels->begin();
            neopixels->setBrightness(64);
            for (int i = 0; i < neopixels->numPixels(); i++) {
                neopixels->setPixelColor(i, BLACK);
            }
            neopixels->show(); // Initialize all pixels to 'off'
        }
        
        
        void set_brightness(int brightness) {
            int b = map(brightness, 0, 100, 16, 196);
            #ifdef PRINTF_DEBUGGER
                Serial.printf("setting brightness to %d\n", b);
            #endif
            neopixels->setBrightness(b);
        }
        
        void paint(int i, color_t color) {
            neopixels->setPixelColor(i, color);
        }


        void paint(Dot* dot) {
            paint(txlate(dot), dot->color);
        }
        
        
        void unpaint(Dot* dot) {
            paint(txlate(dot), BLACK);
        }


        void clear() {
            neopixels->clear();
        }
        
        void show() {
            neopixels->show();
        }
        
        
        void render(Dot** dots) {
            // for (int cursor = first(dots); cursor != -1; cursor = next(cursor, dots)) {
            for (int cursor = 0; cursor < MAX_DOTS; cursor++) {
                if (dots[cursor]->active) {
                    paint(dots[cursor]);
                }
            }
        }
};
 
#endif