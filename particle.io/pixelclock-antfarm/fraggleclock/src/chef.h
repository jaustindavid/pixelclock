#ifndef CHEF_H
#define CHEF_H

/*
 * a Chef makes food in the shape of time.
 */

/*
 xxx  x  xxx  xxx  x x  xxx  xxx  xxx  xxx  xxx
 x x  x    x    x  x x  xx   x      x  x x  xxx
 x x  x  xx    xx  xxx    x  xxx   x   xxx    x
 xxx  x  xxx  xxx    x  xxx  xxx   x   xxx  xxx
 */

#include "dot.h"
#include "list.h"

#undef PRINTF_DEBUGGER

class Chef {
    private:
        int last_hh, last_mm;
        int font[10][5] = {
            // 0
            {0b0110, 
             0b1001,
             0b1001,
             0b1001,
             0b0110}, 
            // 1
            {0b0010,
             0b0010,
             0b0010,
             0b0010,
             0b0010},
            // 2
            {0b1110,
             0b0001,
             0b0110,
             0b1000,
             0b1111},
             // 3
            {0b1110,
             0b0001,
             0b1110,
             0b0001,
             0b1110},
             // 4
            {0b1001,
             0b1001,
             0b1111,
             0b0001,
             0b0001},
             // 5
            {0b1111,
             0b1000,
             0b1110,
             0b0001,
             0b1110},
             // 6
            {0b0110,
             0b1000,
             0b1110,
             0b1001,
             0b0110},
             // 7
            {0b1111,
             0b0001,
             0b0010,
             0b0100,
             0b0100},
             // 8
            {0b0110,
             0b1001,
             0b0110,
             0b1001,
             0b0110},
             // 9
            {0b0110,
             0b1001,
             0b0111,
             0b0001,
             0b0110}
            };


    public:
        Chef() : last_hh(-1), last_mm(-1) {};
        
        void setup() {
            // Particle.function("chef_chaos", &Chef::beChaos, this); 
        }
        
        
        void prepare(Dot* food[], int d, int dx, int dy) {
            #ifdef PRINTF_DEBUGGER
                Serial.printf("Chef: starting render(%d)\n", d);
            #endif
            for (int y = 0; y < 5; y++) {
                #ifdef PRINTF_DEBUGGER
                    Serial.printf("Scanning %01x\n", font[d][y]);
                #endif
                for (int x = 0; x < 4; x++) {
                    if (font[d][y] & (1 << x)) {  // Check if pixel is set
                        // Draw the pixel
                        #ifdef PRINTF_DEBUGGER
                            Serial.printf("found pixel at (%d,%d)\n", 3-x, y);
                        #endif
                        // food->add(new Dot(3-x+dx, y+dy, GREEN));
                        Dot* dot = activate(food);
                        dot->x = 3-x+dx;
                        dot->y = y+dy;
                        dot->color = GREEN;
                    }
                }
            }
        }
        
        
        void decimate(Dot* food[]) {
            Dot* dot = activate(food);
            dot->x = 0;  dot->y = 11; dot->color = GREEN;
            dot = activate(food);
            dot->x = 1;  dot->y = 11;
            dot = activate(food);
            dot->x = 0;  dot->y = 12;
            dot = activate(food);
            dot->x = 1;  dot->y = 12;
        }


        void cook(Dot* food[], int hh, int mm, bool decimal) {
            if (true || (hh != last_hh) || (mm != last_mm)) {
                #ifdef PRINTF_DEBUGGER
                    Serial.printf("Chef: starting cook(%02d, %02d)\n", hh, mm);
                #endif
                last_hh = hh;
                last_mm = mm;
                // food->clear();
                clear_dots(food);
                prepare(food, hh / 10, 3, 1);
                prepare(food, hh % 10, 9, 1);
                prepare(food, mm / 10, 3, 8);
                prepare(food, mm % 10, 9, 8);
                if (decimal) {
                    decimate(food);
                }
                #ifdef PRINTF_DEBUGGER
                    print_list(food);
                #endif
            }
        }
};

#endif
