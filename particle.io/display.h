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
        color_t fg[MATRIX_X*MATRIX_Y], bg[MATRIX_X*MATRIX_Y];
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
        }


        void init(void) {
            neopixels->begin();
            neopixels->setBrightness(64);
            memset(fg, 0, sizeof(fg));
            clear();
        }
        
        
        void set_brightness(int brightness) {
            int b = map(brightness, 0, 100, 16, 196);
            #ifdef PRINTF_DEBUGGER
                Serial.printf("setting brightness to %d\n", b);
            #endif
            neopixels->setBrightness(b);
        }
        
        
        void paint(int i, color_t color) {
            // neopixels->setPixelColor(i, color);
            fg[i] = color;
        }


        void paint(Dot* dot) {
            paint(txlate(dot), dot->color);
        }
        
        
        void unpaint(Dot* dot) {
            paint(txlate(dot), BLACK);
        }


        void clear() {
            memcpy(bg, fg, sizeof(fg));
            memset(fg, 0, sizeof(fg));
            neopixels->clear();
        }
        

        void show() {
            for (int i = 0; i < MATRIX_X * MATRIX_Y; i++) {
                neopixels->setPixelColor(i, fg[i]);
            }
            neopixels->show();
        }
        
        
        color_t wavrgb(color_t a, int weight_a, color_t b, int weight_b) {
            uint8_t ra = (a & RED) >> 16;
            uint8_t rb = (b & RED) >> 16;
            uint8_t ga = (a & GREEN) >> 8;
            uint8_t gb = (b & GREEN) >> 8;
            uint8_t ba = (a & BLUE);
            uint8_t bb = (b & BLUE);
            // Serial.printf("0x%06x: %02x %02x %02x || 0x%06x: %02x %02x %02x", a, ra, ga, ba, b, rb, gb, bb);
            color_t ret = ((ra*weight_a+rb*weight_b)/(weight_a+weight_b) << 16) 
                        | ((ga*weight_a+gb*weight_b)/(weight_a+weight_b) << 8) 
                        | (ba*weight_a+bb*weight_b)/(weight_a+weight_b);
            // Serial.printf(" == 0x%06x\n", ret);
            return ret;
        }

        
        // a multi-pass show(), one pass per show_timer
        void show(SimpleTimer* show_timer) {
            for (int w = 1; w < 5; w++) {
                for (int i = 0; i < MATRIX_X * MATRIX_Y; i++) {
                    neopixels->setPixelColor(i, wavrgb(fg[i], w, bg[i], 5-w));
                }
                neopixels->show();
                show_timer->wait();
            }
        }
        
        
        // render() writes to the fg[] buffer;
        // show() will copy these to pixels, then push them
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