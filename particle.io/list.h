#ifndef LIST_H
#define LIST_H

#include "dot.h"
#include "ant.h"


int len(Dot** dots) {
    int n = 0;
    for (int i = 0; i < MAX_DOTS; i++) {
        if (dots[i]->active) {
            n++;
        }
    }
    return n;
}


/* 
 * for (Dot* dot = first(food); dot != nullptr; dot = next(food)) ...
 * for (int cursor = first(food); cursor != -1; cursor = next(cursor, food)) ...
 */

// returns the NEXT thing after cursor...
int next(int cursor, Dot** dots) {
    for (++cursor; cursor < MAX_DOTS; cursor++) {
        if (dots[cursor]->active) {
            return cursor;
        }
    }
    return -1;
}

int first(Dot** dots) {
    return next(-1, dots);
}


void clear_dots(Dot** dots) {
    for (int i = 0; i < MAX_DOTS; i++) {
        dots[i]->active = false;
    }
}


// finds the first inactive dot, activates it, returns it
Dot* activate(Dot** dots) {
    for (int i = 0; i < MAX_DOTS; i++) {
        if (!dots[i]->active) {
            dots[i]->active = true;
            return dots[i];
        }
    }
    return nullptr;
}


// deactivates the dot at cursor
void deactivate(int cursor, Dot** dots) {
    dots[cursor]->active = false;
}


// true if needle is active in haystack
bool in(Dot* needle, Dot** haystack) {
    for (int i = 0; i < MAX_DOTS; i++) {
        if (haystack[i]->active && needle->equals(haystack[i])) {
            return true;
        }
    }
    return false;
}


// true if needle has any neighbors in haystack
bool any_adjacent(Dot* needle, Dot** haystack) {
    for (int i = 0; i < MAX_DOTS; i++) {
        if (haystack[i]->active && needle->adjacent(haystack[i])) {
            return true;
        }
    }
    return false;
}

/*
// returns the index of a close-ish needle in haystack
int closeish(Dot* needle, Dot** haystack) {
    // find the range of distances
    float closest = 23, furthest = 0, sum_p = 0;
    for (int i = 0; i < MAX_DOTS; i++) {
        float distance = needle->distance_to(haystack[i]));
        closest = min(closest, distance);
        furthest = max(furthest, distance);
        sum_p += 1/(distance+1);
    }
    // favor lower distances 
    while (1) {
        for (int i = 0; i < MAX_DOTS; i++) {
            float r = random(0, RAND_MAX) / (float)RAND_MAX;
            float p = 1/((distance+1)*sum_p);
            if (p > r) {
                return i;
            }
        }
    }
}
*/
#endif