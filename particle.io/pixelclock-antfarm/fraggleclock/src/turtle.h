#ifndef _TURTLE_H_
#define _TURTLE_H_

#include "dot.h"
#include "ant.h"
#include "list.h"

#define TURTLE_SPEED 750 // ms per step or pick/place

#define DEBUG_LEVEL 2

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


        // find the lowest value in distances[][] around (xp, yp)
        byte min_distance(byte xp, byte yp, byte distances[16][16]) {
            byte r = 99;
            for (int i = max(xp - 1, 0); i <= min(xp + 1, 15); i++) {
                for (int j = max(yp - 1, 0); j <= min(yp + 1, 15); j++) {
                    r = min(r, distances[i][j]);
                    #if DEBUG_LEVEL > 3
                    Log.info("peeking (%d,%d); r=%d", i, j, r);
                    #endif
                }
            }
            return r;
        } // byte min_distance(xp, yp, distances)


        // mark the distances adjacent to cursor
        void mark_adjacent(Dot *cursor, 
                           Dot* sandbox[50], 
                           byte distances[16][16]) {
            byte i, j;
            for (i = max(cursor->x - 1, 0); i <= min(cursor->x + 1, 15); i++) {
                for (j = max(cursor->y - 1, 0); j <= min(cursor->y + 1, 15); j++) {
                    #if DEBUG_LEVEL > 3
                    Log.info("marking adjacent: (%d,%d)", i, j);
                    #endif
                    if (!in(i, j, sandbox)) {
                        distances[i][j] = min_distance(i, j, distances) + 1;
                        #if DEBUG_LEVEL > 3
                        Log.info("d(%d,%d) = %d", i, j, distances[i][j]);
                        #endif
                    } else {
                        #if DEBUG_LEVEL > 3
                        Log.info("... in sandbox");
                        #endif
                    }
                }
            }
        } // mark_adjancent(curson, sandbox, distances)


        // if possible, move cursor to an adjacent spot 
        // which has a distance, but not yet visited
        bool nearby_unvisited(Dot *cursor, 
                              byte distances[16][16], 
                              bool visited[16][16]) {
            int i, j;
            for (i = max(cursor->x - 1, 0); i <= min(cursor->x + 1, 15); i++) {
                for (j = max(cursor->y - 1, 0); j <= min(cursor->y + 1, 15); j++) {
                    if (!visited[i][j] && distances[i][j] < 99) {
                        cursor->x = i;
                        cursor->y = j;
                        return true;
                    }
                }
            }
            Log.error("REACHED UNREACHABLE CODE AGAIN");
            #if DEBUG_LEVEL > 3
            print_distances(distances);
            #endif
            // delay(10000);
            return false;
        } // bool nearby_unvisited(cursor, distances, visited)
 



        // print the distances[][] matrix
        void print_distances(byte distances[16][16]) {
            for (int y = 0; y < 16; y++) {
                for (int x = 0; x < 16; x++) {
                    if (distances[x][y] < 99) {
                        Serial.printf(" %02d ", distances[x][y]);
                    } else {
                        Serial.printf(" __ ");
                    }
                }
                Serial.println();
            }
        } // print_distances(distances)


        void print_sandbox(Dot* sandbox[]) {
            for (int y = 0; y < 16; y++) {
                for (int x = 0; x < 16; x++) {
                    Serial.printf(in(x, y, sandbox) ? " ## " : " __ ");
                }
                Serial.println();
            }
        } // print_sandbox(sandbox)


        void visit_cells(byte radius, 
                         Dot* sandbox[],
                         bool visited[16][16],
                         byte distances[16][16]) {
            #if DEBUG_LEVEL > 3
            Log.info("visiting (%d,%d), r=%d", x, y, radius);
            #endif
            int i, j;
            Dot cursor = Dot(x, y, BLACK);
            #if DEBUG_LEVEL > 3
            Log.info("i,j = (%d,%d)", x-radius, y-radius);
            #endif
            for (i = max(0, x - radius); 
                 i <= min(MATRIX_X - 1, x + radius); 
                 i++) {
                for (j = max(0, y - radius); 
                     j <= min(MATRIX_Y - 1, y + radius); 
                     j++) {
                    if (!visited[i][j]) {
                        #if DEBUG_LEVEL > 3
                        Log.info("checking (%d,%d)", i, j);
                        #endif
                        cursor.x = i;
                        cursor.y = j;
                        mark_adjacent(&cursor, sandbox, distances);
                        visited[i][j] = true;
                    } else {
                        #if DEBUG_LEVEL > 3
                        Log.info("not checking (%d,%d)", i, j);
                        #endif
                    }
                }
            }
        } // visit_cells(radius, sandbox, visited, distances)


        // move cursor one step closer to 0
        bool step_home(Dot* cursor, byte distances[16][16]) {
            int i, j;
            for (i = max(cursor->x - 1, 0); i <= min(cursor->x + 1, 15); i++) {
                for (j = max(cursor->y - 1, 0); j <= min(cursor->y + 1, 15); j++) {
                    if (distances[i][j] < distances[cursor->x][cursor->y]) {
                        cursor->x = i;
                        cursor->y = j;
                        return true;
                    }
                }
            }
            Log.error("REACHED UNREACHABLE CODE");
            // delay(5000);
            return false;
        } // step_home(cursor, distances)


    public:
        byte iq = 25;


        // take one step toward the spot, while avoiding everything in sandbox
        bool move_toward(Dot* spot, 
                         Dot* sandbox[],
                         bool junk = true) override {
            Log.info("moving (%d,%d) -> (%d,%d), iq=%d",
                     x, y, spot->x, spot->y, iq);

            if (adjacent(spot)) {
                Log.info("shotcut, jumping to adjacent target");
                x = spot->x;
                y = spot->y;
                return true;
            }
            byte distances[16][16];
            bool visited[16][16];
            int i, j;
            Dot cursor;
            byte radius;

            Log.trace("initializaing distances");
            for (i = 0; i < 16; i++) {
                for (j = 0; j < 16; j++) {
                    distances[i][j] = 99;
                    visited[i][j] = false;
                }
            }

            distances[x][y] = 0;
            cursor.x = x;
            cursor.y = y;

            radius = 1;
            while (radius < iq) {
                #if DEBUG_LEVEL > 2
                Log.info("Checking radius=%d", radius);
                #endif
                visit_cells(radius, sandbox, visited, distances);
                #if DEBUG_LEVEL > 3
                print_distances(distances);
                #endif
                radius ++;
            }

            print_distances(distances);

            if (visited[spot->x][spot->y]) {
                Dot cursor = Dot(spot->x, spot->y, BLACK);
                while (distances[cursor.x][cursor.y] > 1) {
                    if (! step_home(&cursor, distances)) {
                        Log.info("no path home; deferring to Ant::");
                        print_sandbox(sandbox);
                        return Ant::move_toward(spot, sandbox);
                    }
                    Log.info("backtrace: (%d,%d), d=%d", 
                             cursor.x, cursor.y, 
                             distances[cursor.x][cursor.y]);
                }
                x = cursor.x;
                y = cursor.y;
                Log.info("Path found!!! (%d,%d)", x, y);
                return true;
            } 

            Log.info("dest not visited; deferring to Ant::");
            print_sandbox(sandbox);
            return Ant::move_toward(spot, sandbox);
        } // walk_toward(target, sandbox)


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
                    // step_toward(target);
                    move_toward(target, sandbox);
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
                    // step_toward(target);
                    move_toward(target, sandbox);
                }
            }
        }
        
        
        
        void rest(Dot* plan[], Dot* sandbox[]) {
            state = RESTING;
            Log.info("resting.  Looking for tracks in sandbox, not plan");
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
            Log.info("Turtle[%d](%d, %d):%d\n", id, x, y, state);
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
