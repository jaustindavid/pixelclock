#ifndef _TURTLE_H_
#define _TURTLE_H_

#include "dot.h"
#include "ant.h"
#include "list.h"

#define TURTLE_SPEED 750 // ms per step or pick/place

#undef PRINTF_DEBUGGER

/*
    A Turtle is a smarter (?) Fraggle.
   
    if track exists in sandbox but not plan
        state == CLEAN
        clean the closest one
    if a missing spot exists (plan without tracks)
        state == BUILD
        go there & make a track
        
    No restriction on where-to-walk
 */

class Turtle: public Fraggle {
    protected:
        int target_i;
        SimpleTimer* step_timer;
        
        
        int pick_closest_open(Dot* needle[], Dot* haystack[], color_t target_color) {
            int best = -1;
            float best_distance = 99;
            // 1. get the distance to any open food; track the min
            // bool open[MAX_DOTS];
            for (int i = 0; i < MAX_DOTS; i++) {
                if (needle[i]->active && !in(needle[i], haystack) && 
                        ((target_color == BLACK) || (needle[i]->get_color() == target_color))) {
                    #ifdef PRINTF_DEBUGGER
                        Serial.printf("%d (%d,%d) in needles, but not haystack\n", i, needle[i]->x, needle[i]->y);
                    #endif
                    if (best == -1 || distance_to(needle[i]) < best_distance) {
                        best = i;
                        best_distance = distance_to(needle[i]);
                    }
                }
            }
            return best;
        }
        
        
        int pick_closest_open(Dot* needle[], Dot* haystack[]) {
            return pick_closest_open(needle, haystack, BLACK);
        }


        // just scoot
        void step_toward(Dot* target) {
            if (target->x > x) {
                x += 1;
            } else if (target->x < x) {
                x -= 1;
            }
            
            if (target->y > y) {
                y += 1;
            } else if (target->y < y) {
                y -= 1;
            }
        }


    public:
        Turtle() : Fraggle() {
            color = GREEN;
            step_timer = new SimpleTimer(TURTLE_SPEED);
        };

        
        void build(Dot* plan[], Dot* sandbox[]) {
            if (target_i != -1) {
                Dot* target = plan[target_i];
                #ifdef PRINTF_DEBUGGER
                    Serial.printf("T(%d,%d): building to (%d,%d)\n", x, y, target->x, target->y);
                #endif
                if (equals(target)) {
                    #ifdef PRINTF_DEBUGGER
                        Serial.println("placing brick");
                    #endif
                    place_brick(target, RED, sandbox);
                    state = RESTING;
                } else {
                    step_toward(target);
                }
            }
        }
        

        void clean(Dot* plan[], Dot* sandbox[]) {
            if (target_i != -1) {
                Dot* target = sandbox[target_i];
                if (equals(target)) {
                    // pick_up(target_i, sandbox);
                    deactivate(target_i, sandbox);
                    state = RESTING;
                } else {
                    step_toward(target);
                }
            }
        }
        
        
        
        void rest(Dot* plan[], Dot* sandbox[]) {
            state = RESTING;
            #ifdef PRINTF_DEBUGGER
                Serial.println("resting.  Looking for tracks in sandbox, not plan");
            #endif
            int i;
            if ((i = pick_closest_open(sandbox, plan, RED)) != -1) {
                // if tracks not on plan, clean
                #ifdef PRINTF_DEBUGGER
                    Serial.printf("found %d (%d,%d); cleaning\n", i, sandbox[i]->x, sandbox[i]->y);
                #endif
                state = CLEANING;
                target_i = i;
            } else {
                #ifdef PRINTF_DEBUGGER
                    Serial.println("Picking open plan, sandbox to check for open plan spots");
                #endif
                if ((i = pick_closest_open(plan, sandbox)) != -1) {
                    // plan missing tracks; build
                    #ifdef PRINTF_DEBUGGER
                        Serial.printf("found %d (%d,%d); building\n", i, plan[i]->x, plan[i]->y);
                    #endif
                    state = BUILDING;
                    target_i = i;
                } else if (P(10)) {
                    wander(sandbox);
                } else {
                    #ifdef PRINTF_DEBUGGER
                        Serial.println("Avoiding");
                    #endif
                    avoid(sandbox);
                }
            }
        }


        void run(Dot* plan[], Dot* sandbox[]) {
            if (! step_timer->isExpired()) {
                return;
            }
            #ifdef PRINTF_DEBUGGER
                Serial.printf("Turtle[%d](%d, %d):%d\n", id, x, y, state);
            #endif
            color = GREEN;
            switch (state) {
                case SPAZZING:
                    spaz(sandbox);
                    break;
                case BUILDING:
                    build(plan, sandbox);
                    break;
                case CLEANING:
                    clean(plan, sandbox);
                    break;
                case RESTING: 
                default:
                    rest(plan, sandbox);
            }
        }

};

#endif
