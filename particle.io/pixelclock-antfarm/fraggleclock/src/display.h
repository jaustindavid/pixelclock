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

#ifndef MIN_BRIGHTNESS
    #define MIN_BRIGHTNESS 8
#endif

#ifndef MAX_BRIGHTNESS
    #define MAX_BRIGHTNESS 196
#endif


class Display {
    private:
        color_t fg[PIXEL_COUNT], bg[PIXEL_COUNT];
        Adafruit_NeoPixel *neopixels;
        int brightness, brightness_target;

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
        } // Display(neopixels)


        void test_forever() {
          Dot dot;
          dot.color = WHITE;
          for (int x = 0; x < MATRIX_X; x++) {
            for (int y = 0; y < MATRIX_Y; y++) {
              dot.x = x; dot.y = y;
              int i = txlate(&dot);
              Log.trace("(%d,%d)->%d", x, y, i);
              neopixels->setPixelColor(i, WHITE);
              neopixels->show();
              delay(1000);
              neopixels->clear();
              neopixels->show();
            }
          }
        } // test_forever()


        void setup(void) {
            neopixels->begin();
            neopixels->setBrightness(64);
            memset(fg, 0, sizeof(fg));
            clear();
            Particle.variable("display_brightness", this->brightness);
        } // setup()
        
        
        #define HYSTERESIS 2
        int set_brightness(int b) {
            int new_brightness = map(b, 0, 100, MIN_BRIGHTNESS, MAX_BRIGHTNESS);
            if (abs(new_brightness - brightness_target) > HYSTERESIS) {
                brightness_target = new_brightness;
            }
            if (brightness_target > brightness) {
                brightness += 1;
            } else if (brightness_target < brightness) {
                brightness -= 1;
            }
            #ifdef PRINTF_DEBUGGER
                Serial.printf("setting brightness to %d\n", brightness);
            #endif
            neopixels->setBrightness(brightness);
            return brightness;
        } // set_brightness(b)
        
        
        void paint(int i, color_t color) {
            // neopixels->setPixelColor(i, color);
            fg[i] = color;
        } // paint(i, color)


        void paint(Dot* dot) {
            paint(txlate(dot), dot->color);
        } // paint(dot)
        
        
        void unpaint(Dot* dot) {
            paint(txlate(dot), BLACK);
        } // unpaint(dot)


        void clear() {
            memcpy(bg, fg, sizeof(fg));
            memset(fg, 0, sizeof(fg));
            neopixels->clear();
        } // clear()
        

        void show() {
            for (int i = 0; i < PIXEL_COUNT; i++) {
                neopixels->setPixelColor(i, fg[i]);
            }
            neopixels->show();
        } // show()
        
        
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
        } // wavrgb(a, weight, b, weight)

        
        // a multi-pass show(), one pass per show_timer
        void show(SimpleTimer* show_timer) {
            for (int w = 1; w < 5; w++) {
                // unsigned long start = millis();
                for (int i = 0; i < PIXEL_COUNT; i++) {
                    neopixels->setPixelColor(i, wavrgb(fg[i], w, bg[i], 5-w));
                }
                neopixels->show();
                // Serial.printf("%d ms elapsed between shows\n", millis() - start);
                show_timer->wait();
            }
        } // show(timer)
        
        
        // render() writes to the fg[] buffer;
        // show() will copy these to pixels, then push them
        void render(Dot* dots[]) {
            render(dots, MAX_DOTS);
            return;
            for (int cursor = 0; cursor < MAX_DOTS; cursor++) {
                if (dots[cursor]->active) {
                    paint(dots[cursor]);
                }
            }
        } // render(dots)
        

        // renders the first n dots
        // backwards, so the lowest-numbered ones are "top"
        void render(Dot* dots[], int ndots) {
            // for (int cursor = first(dots); cursor != -1; cursor = next(cursor, dots)) {
            // for (int cursor = 0; cursor < ndots; cursor++) {
            for (int cursor = ndots-1; cursor >= 0; cursor--) {
                if (dots[cursor]->active) {
                    paint(dots[cursor]);
                }
            }
        } // render(dots, n)
}; // class Display
 
#endif
