/*
 * A single dot.  It can move around among a list of other dots
 */
 
#ifndef DOT_H
#define DOT_H

#include <math.h>
#include "defs.h"
#include "color.h"


class Dot {
    public:
        byte x, y;
        color_t color;
        bool active;

        Dot(void) : x(0), y(0), color(0), active(false) {}
        Dot(int new_x, int new_y, color_t new_color) : x(new_x), y(new_y), color(new_color) {}

        void set_color(color_t new_color) {
            color = new_color;
            // Serial.printf("Color now == %08x\n", color);
        }

        void print() {
            Serial.printf("(%d,%d):%08x", x, y, color);
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

#endif