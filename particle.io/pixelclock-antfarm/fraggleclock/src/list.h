#ifndef LIST_H
#define LIST_H

#include "dot.h"
#include "ant.h"


int len(Dot* dots[]) {
    int n = 0;
    for (int i = 0; i < MAX_DOTS; i++) {
        if (dots[i]->active) {
            n++;
        }
    }
    return n;
}


/* 
 * for (int cursor = first(food); cursor != -1; cursor = next(cursor, food)) ...
 */

// returns the NEXT thing after cursor...
int next(int cursor, Dot* dots[]) {
    for (++cursor; cursor < MAX_DOTS; cursor++) {
        if (dots[cursor]->active) {
            return cursor;
        }
    }
    return -1;
}


int first(Dot* dots[]) {
    return next(-1, dots);
}


void clear_dots(Dot* dots[]) {
    for (int i = 0; i < MAX_DOTS; i++) {
        dots[i]->active = false;
    }
}


// finds the first inactive dot, activates it, returns it
Dot* activate(Dot* dots[]) {
    for (int i = 0; i < MAX_DOTS; i++) {
        if (!dots[i]->active) {
            dots[i]->active = true;
            return dots[i];
        }
    }
    return nullptr;
}


// deactivates the dot at cursor
void deactivate(int cursor, Dot* dots[]) {
    dots[cursor]->active = false;
}


void deactivate(Dot* needle, Dot* haystack[]) {
    for (int i = 0; i < MAX_DOTS; i++) {
        if (haystack[i]->active && needle->equals(haystack[i])) {
            deactivate(i, haystack);
            return;
        }
    }
}

/*
 * ins
 *
 * in(needle, haystack): 
 *    returns a pointer to the first thing at needle's 
 *    loction which is in the haystack
 *    may return needle!
 *
 * in(needle, haystack, color):
 *   returns a pointer to the first thing at needle's
 *   location of color which is in the haystack
 *   Use this if you want to find something "at needle's location"
 *   but definitely not needle
 *
 * in(x, y, haystack):
 *   return true if anything exists at location (x, y)
 */


// true (ptr) if needle is active in haystack
Dot* in(Dot* needle, Dot* haystack[]) {
    if (! needle) {
      return nullptr;
      Log.trace("in(needle, haystack) called with nullptr, wth");
    }
    Log.trace("checking in(needle, haystack)");
    for (int i = 0; i < MAX_DOTS; i++) {
        if (haystack[i] == nullptr) {
            Log.trace("haystack[%d] == nullptr?", i);
        } else if (haystack[i]->active) {
            if (needle == haystack[i]
                || needle->equals(haystack[i])) {
                return haystack[i];
            }
        }
        // WTH
        // if (needle != haystack[i] 
        //     && haystack[i]->active 
        //     && needle->equals(haystack[i])) {
        //     return haystack[i];
        // }
    }
    return nullptr;
}


// true (ptr) if needle of color is active in haystack
Dot* in(Dot* needle, color_t color, Dot* haystack[]) {
    for (int i = 0; i < MAX_DOTS; i++) {
        if (haystack[i]->active 
            && needle->equals(haystack[i]) 
            && (haystack[i]->get_color() == color)) {
            return haystack[i];
        }
    }
    return nullptr;
} // Dot* in(Dot* needle, color_t color, Dot* haystack[])


// true if (x,y) active in haystack
bool in(int x, int y, Dot* haystack[]) {
    for (int i = 0; i < MAX_DOTS; i++) {
        if (haystack[i]->active 
            && (haystack[i]->x == x) 
            && (haystack[i]->y == y)) {
            return true;
        }
    }
    return false;
} // bool in(int x, int y, Dot* haystack[])


// true if (x,y) active in haystack
Dot* find(int x, int y, Dot* haystack[]) {
    for (int i = 0; i < MAX_DOTS; i++) {
        if (haystack[i]->active 
            && (haystack[i]->x == x) 
            && (haystack[i]->y == y)) {
            return haystack[i];
        }
    }
    return nullptr;
} // Dot* find(int x, int y, Dot* haystack[])


// true if needle has any neighbors in haystack
bool any_adjacent(Dot* needle, Dot* haystack[]) {
    for (int i = 0; i < MAX_DOTS; i++) {
        if (haystack[i]->active && needle->adjacent(haystack[i])) {
            return true;
        }
    }
    return false;
}


void print_list(Dot* haystack[]) {
    for (int cursor = first(haystack); cursor != -1; cursor = next(cursor, haystack)) {
        Serial.printf("%d: (%d,%d); ", cursor, haystack[cursor]->x, haystack[cursor]->y);
    }
    Serial.println();
}


#endif
