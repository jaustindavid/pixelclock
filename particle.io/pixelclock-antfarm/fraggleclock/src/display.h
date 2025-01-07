#pragma once

/*
 * A class to manage a grid of LEDs
 * 
 * it renders dots.
 */

#include <neopixel.h>
#include "defs.h"
#include "color.h"
#include "dot.h"
#include "list.h"

#undef PRINTF_DEBUGGER

#define NITE_BRIGHTNESS 1

class Display {
    private:
        color_t fg[PIXEL_COUNT], bg[PIXEL_COUNT];
        Adafruit_NeoPixel *neopixels;
        int brightness, brightness_target;
        byte min_brightness, // min value for LED brightness; 0-255
             max_brightness, // max value for LED brightness; 0-255
             rotation;       // # 90* CW rotations to apply; 0-3
        bool alignment_mode;

        struct save_data_t {
          byte version,
               min,
               max,
               rota;
        };
        

        void read_eeprom() {
          struct save_data_t datum;
          EEPROM.get(DISPLAY_ADDY, datum);
          if (datum.version == 0) {
            min_brightness = datum.min;
            max_brightness = datum.max;
            rotation = datum.rota;
          }
        } // read_eeprom()


        void write_eeprom() {
          struct save_data_t datum;
          datum.version = 0;
          datum.min = min_brightness;
          datum.max = max_brightness;
          datum.rota = rotation;
          EEPROM.put(DISPLAY_ADDY, datum);
        } // write_eeprom()


        // a Particle.function to enable alignment_mode
        int align_me(String data) {
          alignment_mode = !alignment_mode;
          return alignment_mode ? 1 : 0;
        } // int align_me(data)


        // honors alignment_mode: shows dots in the corners
        void maybe_show_alignment() {
          if (alignment_mode) {
            neopixels->setPixelColor(trans_rotate(0,0), WHITE);
            neopixels->setPixelColor(trans_rotate(0, MATRIX_Y-1), WHITE);
            for (byte y = 0; y < MATRIX_Y; y++) {
              neopixels->setPixelColor(trans_rotate(MATRIX_X-1, y), WHITE);
            }
          }
        } // maybe_show_alignment()


        // a Particle.function to set min brightness, and store it
        int set_min_brightness(String data) {
          int new_min_brightness = data.toInt();
          if (new_min_brightness) {
            min_brightness = new_min_brightness;
            write_eeprom();
          }
          return min_brightness;
        } // int set_min_brightness(data)


        // a Particle.function to set max brightness, and store it
        int set_max_brightness(String data) {
          int new_max_brightness = data.toInt();
          if (new_max_brightness) {
            max_brightness = new_max_brightness;
            write_eeprom();
          }
          return max_brightness;
        } // int set_max_brightness(data)


        int set_rotation(String data) {
          rotation = (rotation + 1) % 4;
          write_eeprom();
          return rotation;
        } // set_rotation(junk)


        // returns a rotated + translated (x, y) -> [i]
        int trans_rotate(int x, int y) {
          int xprime = x, 
              yprime = y;

          #if (ASPECT_RATIO == SQUARE)
            // apply a 90* CCW rotation to (xprime, yprime)
            for (int i = 0; i < rotation; i++) {
              int y0 = yprime;
              yprime = xprime;
              xprime = MATRIX_X - 1 - y0;
            }
          #else
            // 180* only
            if (rotation % 2 == 1) {
              xprime = MATRIX_X - 1 - x;
              yprime = MATRIX_Y - 1 - y;
            }
          #endif

          return txlate(xprime, yprime);
        } // int trans_rotate(x, y)


    public:
        Display(Adafruit_NeoPixel *new_neopixels) {
            neopixels = new_neopixels;
            // sensible defaults
            alignment_mode = false;
            min_brightness = 4;
            max_brightness = 64; 
            rotation = 0;
            read_eeprom();
        } // Display(neopixels)


        void test_forever() {
          for (int x = 0; x < MATRIX_X; x++) {
            for (int y = 0; y < MATRIX_Y; y++) {
              int i = txlate(x, y);
              Log.trace("(%d,%d)->%d", x, y, i);
              neopixels->setPixelColor(i, WHITE);
              neopixels->show();
              delay(1000);
              neopixels->clear();
              neopixels->show();
            }
          }
        } // test_forever()


        void setup() {
            neopixels->begin();
            neopixels->setBrightness(64);
            memset(fg, 0, sizeof(fg));
            clear();
            show();
        } // setup();


        void setup_cloud() {
            Particle.variable("display_brightness", this->brightness);
            Particle.function("display_aligner", &Display::align_me, this);
            Particle.function("display_min_brightness", 
                &Display::set_min_brightness, this);
            Particle.function("display_max_brightness", 
                &Display::set_max_brightness, this);
            Particle.function("display_rotate", 
                &Display::set_rotation, this);
        } // setup()
        
        
        #define HYSTERESIS 2 // just to keep it from 'hunting'
        int set_brightness(int b) {
            int new_brightness = map(b, 0, 100, min_brightness, max_brightness);
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
            paint(trans_rotate(dot->x, dot->y), dot->color);
        } // paint(dot)
        
        
        void clear() {
            memcpy(bg, fg, sizeof(fg));
            memset(fg, 0, sizeof(fg));
            neopixels->clear();
        } // clear()
        

        // one-shot show()
        void show() {
            for (int i = 0; i < PIXEL_COUNT; i++) {
                neopixels->setPixelColor(i, fg[i]);
            }
            maybe_show_alignment();
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

        
        // a multi-pass show()
        // REDRAWS_PER_FRAME to transition from bg -> fg, 
        // with REDRAW_SPEED_MS delay between
        // returns the total amount of time waited
        void show_multipass() {
            static SimpleTimer redraw_timer(REDRAW_SPEED_MS);
            for (int w = 1; w < (REDRAWS_PER_FRAME+1); w++) {
                // unsigned long start = millis();
                for (int i = 0; i < PIXEL_COUNT; i++) {
                    neopixels->setPixelColor(i, wavrgb(fg[i], w, bg[i], 
                          REDRAWS_PER_FRAME-w));
                }
                maybe_show_alignment();
                neopixels->show();
                // Serial.printf("%d ms elapsed between shows\n", millis() - start);
                redraw_timer.wait();
            }
        } // show_multipass()
        
        
        // show within a budget
        // returns the actual number of frames shown
        int show(int budget_ms) {
          // compute a sensible # frames and # redraws to fit in budget_ms
          int n_redraws = budget_ms/REDRAW_SPEED_MS + 1;
          int budget_per_frame = budget_ms / n_redraws;
          static SimpleTimer redraw_timer(budget_per_frame);

          for (int w = 1; w <= n_redraws; w++) {
            for (int i = 0; i < PIXEL_COUNT; i++) {
                neopixels->setPixelColor(i, wavrgb(fg[i], w, 
                                                   bg[i], n_redraws-w));
            }
            maybe_show_alignment();
            neopixels->show();
            if (w < n_redraws) {
              redraw_timer.wait();
            }
          }
          return n_redraws;
        } // show(budget_ms)


        // render() writes to the fg[] buffer;
        // show() will copy these to pixels, then push them
        void render(Dot* dots[]) {
            render(dots, MAX_DOTS);
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

   
        // renders dots[] in nitetime mode
        // always takes 1 second
        void nite_render(Dot* dots[]) {
          static SimpleTimer redraw_timer(1000);
          neopixels->setBrightness(NITE_BRIGHTNESS);
          for (int cursor = MAX_DOTS-1; cursor >= 0; cursor--) {
            if (dots[cursor]->active) {
              neopixels->setPixelColor(
                   trans_rotate(dots[cursor]->x, dots[cursor]->y), 
                   NITE_COLOR);
            }
          }
          neopixels->show();
          redraw_timer.wait();
        } // nite_render(dots)


}; // class Display
