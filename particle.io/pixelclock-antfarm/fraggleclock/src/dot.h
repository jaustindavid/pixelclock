#pragma once

/*
 * A single dot.  It can move around among a list of other dots
 */
 
#include <math.h>
#include "defs.h"
#include "color.h"


class Dot {
    public:
        static int next_id;
        int id;
        byte x, y;
        color_t color;
        bool active;

        Dot(void) : x(0), y(0), color(0), active(false) {
            id = Dot::next_id++;
        }
        Dot(int new_x, int new_y, color_t new_color) : x(new_x), y(new_y), color(new_color), active(true) {
            id = Dot::next_id++;
        }


        color_t get_color() {
            return color;
        }
        
        
        void set_color(color_t new_color) {
            color = new_color;
            // Serial.printf("Color now == %08x\n", color);
        }


        // lighten my color by amt; r-amt | g-amt | b-amt
        void lighten(byte amt) {
            uint8_t r = (color & RED) >> 16;
            uint8_t g = (color & GREEN) >> 8;
            uint8_t b = (color & BLUE);
            Log.trace("lighten(%d): before: (%d,%d,%d)", amt, r, g, b);
            r = max(0, r-amt);
            g = max(0, g-amt);
            b = max(0, b-amt);
            color = (r<<16) | (g<<8) | (b);
            Log.trace("; after: (%ld,%ld,%ld)", 
                        (color & RED) >> 16, 
                        (color & GREEN) >> 8, 
                        (color & BLUE));
        }
        

        // "breathes" color based on time
        color_t breathe(const color_t source) {
          int ms = millis() % 2000;
          if (ms > 1000) {
            return source/2;
          }
          return source;
        } // color_t breathe(source)


        void print() {
            Serial.printf("(%d,%d):%08lx", x, y, color);
        }
        
        
        bool equals(Dot*);
        bool adjacent(Dot*);
        float distance_to(Dot*);
};


bool Dot::equals(Dot* other) {
    return other->x == x && other->y == y;
}


// true if other is adjacent to me, but NOT me
bool Dot::adjacent(Dot* other) {
    return ! equals(other)
            && abs(other->x - x) <= 1
            && abs(other->y - y) <= 1;
}


// the x/y cartesian distance to other
float Dot::distance_to(Dot* other) {
    return sqrt((x-other->x)*(x-other->x)+(y-other->y)*(y-other->y));
}
