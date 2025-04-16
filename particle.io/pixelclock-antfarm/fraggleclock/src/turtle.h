#ifndef _TURTLE_H_
#define _TURTLE_H_

#include "dot.h"
#include "ant.h"
#include "list.h"

#if defined(MEGA)
    #define TURTLE_SPEED 375 // ms per step or pick/place
#else
    #define TURTLE_SPEED 750 // ms per step or pick/place
#endif

#if defined(TESTING)
    #define DEBUG_LEVEL 2
#else
    #undef DEBUG_LEVEL
#endif

/*
    A Turtle is a smarter (?) Ant
   
    if track exists in sandbox but not plan
        state == CLEAN
        clean the closest one
    if a missing spot exists (plan without tracks)
        state == BUILD
        go there & make a track
        
    No restriction on where-to-walk
 */

#define RESTING  0
#define FETCHING 1
#define BUILDING 2
#define CLEANING 3
#define DUMPING  4

#define BRICK_COLOR         TIME_COLOR
#define UNUSED_BRICK_COLOR  ALT_COLOR

class Turtle: public Ant {
    protected:
        int target_i;
        SimpleTimer* step_timer;
        uint8_t state, prev_state;
        byte _distances[MATRIX_X][MATRIX_Y];
        bool _visited[MATRIX_X][MATRIX_Y];
        byte target_x, target_y;

        #if defined(TESTING)
            #define DEBUG_DISTANCES 2
        #endif
        // initialize the _distances to target
        void init_distances(Dot* target, Dot* sandbox[]) {
            byte radius, ix, iy;

            #if defined(DEBUG_DISTANCES)
                Log.trace("initializaing distances");
            #endif
            for (ix = 0; ix < MATRIX_X; ix++) {
                for (iy = 0; iy < MATRIX_Y; iy++) {
                    _distances[ix][iy] = 99;
                    _visited[ix][iy] = false;
                }
            }

            _distances[target->x][target->y] = 0;

            #if DEBUG_DISTANCES >= 2
                Log.info("marking radii; (%d,%d)<-(%d,%d)",
                         x, y, target->x, target->y);
            #endif
            radius = 1;
            while (radius < iq && (_distances[x][y] == 99)) {
                visit_cells_from_target(target, radius, sandbox);
                #if DEBUG_DISTANCES >= 3
                    Log.info("r=%d:", radius);
                    print_distances(_distances, target->x, target->y);
                #endif
                radius ++;
            }
            
            #if DEBUG_DISTANCES >= 1
                Log.info("Distances, with radii:");
                print_distances(_distances, target->x, target->y);
            #endif
        } // init_distances()


        bool reachable(Dot* target, Dot* sandbox[]) {
            Log.trace("reachable(%d,%d) ?", target->x, target->y);
            // if target is new, re-initialize
            if ((target_x != target->x) 
                || (target_y != target->y)) {
              target_x = target->x;
              target_y = target->y;
              init_distances(target, sandbox);
            }

            #if defined(TESTING)
                if (_distances[x][y] >= 99) {
                    Serial.printf("not reachable from (%d,%d) -> (%d,%d)\n",
                                  x, y, target->x, target->y);
                    print_distances(_distances, target->x, target->y);
                    Serial.print("visited? ");
                    Serial.println(_visited[x][y]?"y":"n");
                }
            #endif

            return _distances[x][y] < 99;
        } // bool reachable(target, sandbox)


        #if defined(TESTING)
            #define DEBUG_VISIT 0
        #endif
        void visit_cells_from_target(Dot* target,
                                     byte radius,
                                     Dot* sandbox[]) {
            #if DEBUG_VISIT >= 1
                Log.info("visiting (%d,%d), r=%d", 
                         target->x, target->y, radius);
                Log.info("CHECK d[%d,%d] = %d", 
                         target->x, target->y, 
                         _distances[target->x][target->y]);
            #endif
            int ix, iy;
            Dot cursor = Dot(target->x, target->y, BLACK);
            #if DEBUG_LEVEL > 3
                Log.info("ix,iy = (%d,%d)", 
                         target->x-radius, target->y-radius);
            #endif
            for (ix = max(0, target->x - radius); 
                 ix <= min(MATRIX_X - 1, target->x + radius); 
                 ix++) {
                for (iy = max(0, target->y - radius); 
                     iy <= min(MATRIX_Y - 1, target->y + radius); 
                     iy++) {
                    if (!_visited[ix][iy]) {
                        #if DEBUG_LEVEL > 3
                            Log.info("checking (%d,%d)", ix, iy);
                        #endif
                        cursor.x = ix;
                        cursor.y = iy;
                        mark_adjacent(&cursor, sandbox, _distances);
                        _visited[ix][iy] = true;
                    } else {
                        #if DEBUG_LEVEL > 3
                            Log.info("not checking (%d,%d)", ix, iy);
                        #endif
                    }
                }
            }
        } // visit_cells_from_target(target, radius, sandbox)


        #if defined(TESTING)
            #define DEBUG_STUMBLE 0
        #endif
        // scoot cursor one step closer to zero
        bool stumble_downhill2() {
            int ix, iy, ntries;
            #if DEBUG_STUMBLE >= 2
                Log.info("stumbling downhill from (%d,%d)->(%d,%d)", 
                         x, y, target_x, target_y);
            #endif
            #if DEBUG_STUMBLE >= 3
                print_distances(_distances, target_x, target_y);
            #endif
            ix = rand_x(x);
            iy = rand_y(y);
            ntries = 0;
            while ((_distances[ix][iy] >= _distances[x][y]) && ntries < 31) {
              ix = rand_x(x);
              iy = rand_y(y);
              ntries++;
            }
            if (_distances[ix][iy] < _distances[x][y]) {
              x = ix;
              y = iy;
              #if DEBUG_STUMBLE >= 2
                  Log.info("stumbled to (%d,%d)", x, y);
              #endif
              return true;
            }
            #if DEBUG_STUMBLE >= 1
                Log.warn("ntries: %d", ntries);
                Log.warn("I am (%d,%d) d=%d", x, y, _distances[x][y]);
                print_distances(_distances, x, y);
            #endif
            for (ix = max(x-1, 0); ix <= min(x+1, MATRIX_X-1); ix++) {
                for (iy = max(y-1, 0); iy <= min(y+1, MATRIX_Y-1); iy++) {
                    if (_distances[ix][iy] < _distances[x][y]) {
                        x = ix;
                        y = iy;
                        #if DEBUG_STUMBLE >= 1
                            Log.info("2ND TRY stumbled to (%d,%d)", x, y);
                        #endif
                        return true;
                    } else {
                        #if DEBUG_STUMBLE >= 1
                            Log.info("rejecting (%d,%d) d=%d",
                                     ix, iy, _distances[ix][iy]);
                        #endif
                    }
                } // for (iy)
            } // for (ix)
            Log.warn("REACHED UNREACHABLE CODE: failed to stumble downhill");
            return false;
        } // bool stumble_downhill(cursor, distances)
          

        // utilize Djikstra's algorithm and clever caching
        // to move one step from (x,y)->target
        bool move_djikstra(Dot* target, Dot* sandbox[]) {
            // shortcut: already there?
            if (adjacent(target) || equals(target)) {
                Log.info("shortcut, jumping to adjacent target");
                x = target->x;
                y = target->y;
                return true;
            }

            // if target is new, re-initialize
            if ((target_x != target->x) 
                || (target_y != target->y)) {
              target_x = target->x;
              target_y = target->y;
              init_distances(target, sandbox);
            }

            // do we have a solution?
            if (_visited[x][y] || (_distances[x][y] < iq)) {
                return stumble_downhill2() or jump(target, sandbox);
            } else {
                Log.warn("no djikstra solution :/");
                print_distances(_distances, target_x, target_y);
                return jump(target, sandbox);
            }
        } // move_djikstra(target, sandbox)


        bool is_brick(Dot* target) {
          return (target
                  && ((target->get_color() == BRICK_COLOR)
                      || (target->get_color() == UNUSED_BRICK_COLOR)));
        } // bool is_brick(target)


        // picks up a brick in sandbox (removes it)
        void pick_up(Dot* brick, Dot* sandbox[]) {
          deactivate(brick, sandbox);
        } // pick_up(brick, sandbox)


        // creates ("places") a brick of color at target in sandbox
        void place_brick(Dot* target, color_t color, Dot* sandbox[]) {
          Dot* brick = activate(sandbox);
          brick->set_color(color);
          brick->x = target->x;
          brick->y = target->y;
        } // place_brick(target, color, sandbox[])
        

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
        byte min_distance(int xp, int yp, byte distances[MATRIX_X][MATRIX_Y]) {
            byte r = 99;
            for (int i = max(xp - 1, 0); i <= min(xp + 1, MATRIX_X-1); i++) {
                for (int j = max(yp - 1, 0); j <= min(yp + 1, MATRIX_Y-1); j++) {
                    r = min(r, distances[i][j]);
                    #if DEBUG_LEVEL > 3
                    Log.info("peeking (%d,%d); r=%d", i, j, r);
                    #endif
                }
            }
            if (r > distances[xp][yp]) {
              Log.warn("!!! MIN DISTANCES: r=%d but d[%d,%d]=%d",
                       r, xp, yp, distances[xp][yp]);
            }
            return r;
        } // byte min_distance(xp, yp, distances)


        #if defined(TESTING)
            #define DEBUG_MARK 1
        #endif
        // mark the distances adjacent to cursor
        void mark_adjacent(Dot *cursor, 
                           Dot* sandbox[], 
                           byte distances[MATRIX_X][MATRIX_Y]) {
            int i, j;
            for (i = max((int)(cursor->x - 1), 0); 
                 i <= min((int)(cursor->x + 1), MATRIX_X-1); i++) {
                for (j = max((int)(cursor->y - 1), 0); 
                     j <= min((int)(cursor->y + 1), MATRIX_Y-1); j++) {
                    #if DEBUG_MARK >= 3
                    Log.info("marking adjacent: (%d,%d)", i, j);
                    #endif
                    if (!in(i, j, sandbox) 
                        || (i==x && j==y)) {
                        distances[i][j] = min(distances[i][j], 
                                    min_distance(i, j, distances) + 1);
                        #if DEBUG_MARK >= 3
                        Log.info("d(%d,%d) = %d", i, j, distances[i][j]);
                        #endif
                    } else {
                        #if DEBUG_MARK >= 3
                        Log.info("... in sandbox");
                        #endif
                    }
                    #if DEBUG_MARK >= 1
                        if (i==x && j==y) {
                            Log.info("ME(%d,%d) = %d", i, j, distances[i][j]);
                        }
                    #endif
                }
            }
        } // mark_adjancent(curson, sandbox, distances)


        // if possible, move cursor to an adjacent spot 
        // which has a distance, but not yet visited
        bool nearby_unvisited(Dot *cursor, 
                              byte distances[MATRIX_X][MATRIX_Y], 
                              bool visited[MATRIX_X][MATRIX_Y]) {
            int i, j;
            for (i = max(cursor->x - 1, 0); 
                 i <= min(cursor->x + 1, MATRIX_X-1); i++) {
                for (j = max(cursor->y - 1, 0); 
                     j <= min(cursor->y + 1, MATRIX_Y-1); j++) {
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
        void print_distances(byte distances[MATRIX_X][MATRIX_Y],
                             int px = -1, int py = -1) {
            for (int iy = 0; iy < MATRIX_Y; iy++) {
                for (int ix = 0; ix < MATRIX_X; ix++) {
                    if (ix == px && iy == py) {
                        Serial.printf("[] ");
                    } else if (ix == x && iy == y) {
                        Serial.printf("<> ");
                    } else if (distances[ix][iy] < 99) {
                        Serial.printf("%02d ", distances[ix][iy]);
                    } else {
                        Serial.printf(",, ");
                    }
                }
                Serial.println();
            }
        } // print_distances(distances)


        void visit_cells(byte radius, 
                         Dot* sandbox[],
                         bool visited[MATRIX_X][MATRIX_Y],
                         byte distances[MATRIX_X][MATRIX_Y]) {
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


        #undef DEBUG_STUMBLE
        // scoot cursor one step closer to zero
        bool stumble_downhill(Dot* cursor, 
                              byte distances[MATRIX_X][MATRIX_Y]) {
            byte hill;
            int ix, iy;
            #ifdef DEBUG_STUMBLE
                Log.info("stumbling downhill from (%d,%d)", 
                         cursor->x, cursor->y);
                print_distances(distances, cursor->x, cursor->y);
            #endif
            ix = rand_x(cursor->x);
            iy = rand_y(cursor->y);
            hill = distances[cursor->x][cursor->y];
            while (distances[ix][iy] >= hill) {
              ix = rand_x(cursor->x);
              iy = rand_y(cursor->y);
            }
            if (distances[ix][iy] < distances[cursor->x][cursor->y]) {
              cursor->x = ix;
              cursor->y = iy;
              Log.info("stumbled to (%d,%d)", cursor->x, cursor->y);
              return true;
            }
            Log.warn("REACHED UNREACHABLE CODE: failed to stumble downhill");
            print_distances(distances, cursor->x, cursor->y);
            return false;
        } // book stumble_downhill(cursor, distances)


        // move cursor one step closer to 0, but ... shuffle
        bool shuffle_home(Dot* cursor, byte distances[MATRIX_X][MATRIX_Y]) {
          int i, j, k;
          Log.info("shuffling home from (%d,%d)", 
                   cursor->x, cursor->y);
          for (k = 0; k < 5; k++) {
            for (i = max(cursor->x - 1, 0); 
                 i <= min(cursor->x + 1, MATRIX_X-1); i++) {
                for (j = max(cursor->y - 1, 0); 
                     j <= min(cursor->y + 1, MATRIX_Y-1); j++) {
                    if (distances[i][j] < distances[cursor->x][cursor->y]
                        && P(35)) {
                        cursor->x = i;
                        cursor->y = j;
                        return true;
                    }
                }
            }
          }
          Log.info("no path found while shuffling home...");
          Log.info("Stuck at (%d,%d)", 
                   cursor->x, cursor->y);
          print_distances(distances, cursor->x, cursor->y);
          return false;
        } // shuffle_home(cursor, distances)


    public:
        byte iq;    // smartness; also the max search radius, tiles
        int budget; // ms; time allowed to search before wandering


        // take one step toward the spot, while avoiding everything in sandbox
        // djikstra rollin over
        // 1. create a huge map of high distances
        // 2. "visit" every tile and decrement the distance from here to there
        // 3. if the destination was visited, a path exists
        // 4. backtrace from the dest, stepping in lowest-numbered tiles
        // 5. the best next step is the last one to "me"
        bool move_toward(Dot* spot, 
                         Dot* sandbox[],
                         bool junk = true) override {
            SimpleTimer move_budget(budget); // max time allowed to find a move
            
            Log.info("moving (%d,%d) -> (%d,%d), iq=%d",
                     x, y, spot->x, spot->y, iq);

            if (adjacent(spot) || equals(spot)) {
                Log.info("shortcut, jumping to adjacent target");
                x = spot->x;
                y = spot->y;
                return true;
            }
            byte distances[MATRIX_X][MATRIX_Y];
            bool visited[MATRIX_X][MATRIX_Y];
            int i, j;
            Dot cursor;
            byte radius;

            Log.trace("initializaing distances");
            for (i = 0; i < MATRIX_X; i++) {
                for (j = 0; j < MATRIX_Y; j++) {
                    distances[i][j] = 99;
                    visited[i][j] = false;
                }
            }
            #if DEBUG_LEVEL > 3
                Log.info("Distances, initialized:");
                print_distances(distances);
            #endif

            distances[x][y] = 0;
            cursor.x = x;
            cursor.y = y;

            Log.trace("marking radii");
            radius = 1;
            while (radius < iq) { // && ! move_budget.isExpired()) {
                #if DEBUG_LEVEL > 2
                Log.trace("Checking radius=%d", radius);
                #endif
                visit_cells(radius, sandbox, visited, distances);
                #if DEBUG_LEVEL > 3
                Log.info("one more radius:");
                print_distances(distances);
                #endif
                radius ++;
            }
            
            #if DEBUG_LEVEL >= 3
                Log.info("Distances, with radii:");
                print_distances(distances);
            #endif

            Log.trace("Checking for a path home");
            if (visited[spot->x][spot->y]) {
                Log.trace("visited! (a path to (%d,%d) is possible)",
                          spot->x, spot->y);
                Dot cursor = Dot(spot->x, spot->y, BLACK);
                while (distances[cursor.x][cursor.y] > 1) {
                  if (!stumble_downhill(&cursor, distances)) {
                    Log.warn("OMG this should not happen; jumpin");
                    return jump(spot, sandbox);
                  }
                }
                /*
                while (distances[cursor.x][cursor.y] > 1) {
                  Log.info("cursor(%d,%d); distance %d",
                           cursor.x, cursor.y, distances[cursor.x][cursor.y]);
                    if (! shuffle_home(&cursor, distances)) {
                        // || move_budget.isExpired()) {
                        // Log.info("no path home; deferring to Ant::");
                        // print_sandbox(sandbox);
                        // return Ant::move_toward(spot, sandbox);
                        Log.warn("failed to shuffle home; jumping (%d,%d)->(%d,%d)",
                                 x, y, spot->x, spot->y);
                        print_distances(distances);
                        Serial.println();
                        return jump(spot, sandbox);
                    }
                    Log.info("backtrace: (%d,%d), d=%d", 
                             cursor.x, cursor.y, 
                             distances[cursor.x][cursor.y]);
                }
                */
                x = cursor.x;
                y = cursor.y;
                Log.trace("Path found!!! Moved to (%d,%d)", x, y);
                return true;
            } 

            Log.warn("dest not visited; deferring to Ant::");
            // print_sandbox(sandbox);
            return Ant::move_toward(spot, sandbox);
        } // move_toward(target, sandbox)


        Turtle() : Ant() {
            active = true;
            step_timer = new SimpleTimer(TURTLE_SPEED);
            iq = MAX_IQ;
            budget = 250; // ms
            target_x = target_y = 99;
        } // Turtle()

        
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
                    place_brick(target, BRICK_COLOR, sandbox);
                    state = RESTING;
                } else {
                    /*
                    if (! move_toward(target, sandbox)) {
                      Log.warn("failed to move toward, in Turtle::build");
                    }
                    */
                    if (!move_djikstra(target, sandbox)) {
                        Log.warn("failed to move, in Turtle::build");
                    }
                }
            }
        } // build(plan, sandbox)
        

        void clean(Dot* plan[], Dot* sandbox[]) {
            if (target_i != -1) {
                Dot* target = sandbox[target_i];
                if (equals(target)) {
                    deactivate(target_i, sandbox);
                    state = RESTING;
                } else {
                    /*
                    move_toward(target, sandbox);
                    if (! move_toward(target, sandbox)) {
                      Log.warn("failed to move toward, in Turtle::clean");
                    }
                    */
                    if (!move_djikstra(target, sandbox)) {
                        Log.warn("failed to move, in Turtle::clean");
                    }
                }
            }
        } // clean(plan, sandbox)
        
        
        
        void rest(Dot* plan[], Dot* sandbox[]) {
            state = RESTING;
            Log.info("resting.  Looking for tracks in sandbox, not plan");
            int i;
            if ((i = pick_closest_open(sandbox, plan, BRICK_COLOR)) != -1) {
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
            color = SPRITE_COLOR; // GREEN;
            switch (state) {
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


#define NTURTLES 1

void setup_turtles(Dot* sandbox[]) {
    #ifdef PRINTF_DEBUGGER
        Serial.println("Making turtle(s)");
    #endif
    for (int i = 0; i < MAX_DOTS; i++) {
        if (i < NTURTLES) {
            sandbox[i] = new Turtle();
        } else {
            sandbox[i] = new Dot();
        }
    }
} // setup_turtles(sandbox)


void loop_turtles(Dot* food[], Dot* sandbox[]) {
    for (int i = 0; i < NTURTLES; i++) {
        Turtle* turtle = (Turtle*)sandbox[i];
        turtle->run(food, sandbox);
    }
} // loop_turtles(food, sandbox)


#endif
