#ifndef ANT_H
#define ANT_H

#include "dot.h"
#include "list.h"

#undef PRINTF_DEBUGGER

#define P(M) (random(100) < M)

class Ant : public Dot {
    public:
        Ant() : Dot() {
            color = WHITE;
        };


        // tries to jump to, or adjacent to, target
        bool jump(Dot* target, Dot* sandbox[]) {
            Log.warn("JUMPING!!! (%d,%d)", x, y);
            Dot proxy = Dot(target->x, target->y, WHITE);
            int i = 0;
            while (i < 16) {
                if(randomize(&proxy, sandbox)) {
                    if (!in(&proxy, sandbox)) {
                        x = proxy.x;
                        y = proxy.y;
                        Log.warn("JUMPED!!! (%d,%d)", x, y);
                        return true;
                    }
                }
                i++;
            }
            Log.warn("jump failed");
            return false;
        }
        
        
        // adjust candidate to be a cell adjacent to me and not otherwise
        // represented in sandbox.  returns True if such exists
        virtual bool randomize(Dot* candidate, Dot* sandbox[]) {
            int i = 0;
            while (i < 16) {
                candidate->x = constrain(x+random(-1, 2), 0, MATRIX_X-1);
                candidate->y = constrain(y+random(-1, 2), 0, MATRIX_Y-1);
                 if (!in(candidate, sandbox)) { 
                    return true;
                 }
                 i++;
            }
            return false;
        }


        void wander(Dot* sandbox[]) {
            Dot candidate = Dot(x, y, color);
            if (randomize(&candidate, sandbox)) {
                x = candidate.x;
                y = candidate.y;
            }
        }


        // returns true if it was able to step
        bool step(int dx, int dy, Dot* sandbox[]) {
            Dot proto = Dot(constrain(x+dx, 0, MATRIX_X-1), constrain(y+dy, 0, MATRIX_Y-1), color);
            if (! proto.equals(this) && !in(&proto, sandbox)) {
                x = proto.x;
                y = proto.y;
                return true;
            }
            return false;
        }
        

        // attempts to step to an adjacent location with open food
        // returns true on success
        bool step_adjacent_open(Dot* food[], Dot* sandbox[]) {
            Dot candidate = Dot(x, y, color);
            for (int i = 0; i < 10; i++) {
                if (randomize(&candidate, sandbox) 
                    && in(&candidate, food)) {
                    x = candidate.x;
                    y = candidate.y;
                    return true;
                }
            }
            return false;
        }
        
        
        // got this from Bard ~ 50/50 [edit: less, the first pass was bad]
        // find a nearby "needle" of target_color in a cell which is not occupied in "sandbox"
        int pick_closeish_open(Dot* needle[], Dot* haystack[], 
                                color_t target_color) {
            // 1. find open food
            bool open[MAX_DOTS];
            // Serial.print("open needles: ");
            float sum_of_distance = 0.0;
            for (int i = 0; i < MAX_DOTS; i++) {
                if (target_color != BLACK) {
                    open[i] = needle[i]->active && !in(needle[i], haystack) && (needle[i]->get_color() == target_color);
                } else {
                    open[i] = needle[i]->active && !in(needle[i], haystack);
                }
                if (open[i]) {
                    sum_of_distance += distance_to(needle[i]);
                    // Serial.printf("%d: (%d,%d)@%.2f; ", i, needle[i]->x, needle[i]->y, distance_to(needle[i]));
                }
            }
            // Serial.printf("\nSum: %.1f", sum_of_distance);
            
            float normalizedDistances[MAX_DOTS];
            for (int i = 0; i < MAX_DOTS; i++) {
                if (open[i]) {
                    normalizedDistances[i] = distance_to(needle[i]) / sum_of_distance;
                }
            }
            float randomValue = random(0, RAND_MAX) / (float)RAND_MAX;
            float accumulatedProbability = 0.0;
            for (int i = 0; i < MAX_DOTS; i++) {
                if (open[i]) {
                    accumulatedProbability += normalizedDistances[i];
                    // Serial.printf("%.2f <> %.2f? ", randomValue, accumulatedProbability);
                    if (randomValue <= accumulatedProbability) {
                        // Serial.printf("found %d\n", i);
                        return i;
                    }
                }
            }
            // Serial.println("; none found");
            // FALLTHROUGH / failed to find any point
            return -1;
        }


        int pick_closeish_open(Dot* needle[], Dot* haystack[]) {
            return pick_closeish_open(needle, haystack, BLACK);
        }
        

        int d(int src, int dst) {
            if (dst > src && P(75)) {
                Log.trace("d(%d,%d)-> 1", src, dst);
                return 1;
            } else if (src >= dst && P(75)) {
                Log.trace("d(%d,%d)-> -1", src, dst);
                return -1;
            }
            Log.trace("d(%d,%d)-> 0", src, dst);
            return 0;
        }


        // attempt to move toward spot among the sandbox
        // true if successful
        virtual bool move_toward(Dot* spot, 
                                 Dot* sandbox[], 
                                 bool walls_are_blocking = true) {
            Log.info("moving from (%d,%d)->(%d,%d)", x, y, spot->x, spot->y);
            Log.trace("walls? %c", walls_are_blocking ? 'y':'n');
            int i = 0, dx = 0, dy = 0;
            Dot proto = Dot();
            while (i < 8) {
                dx = d(x, spot->x);
                dy = d(y, spot->y);
                Log.trace("dx,dy = %d,%d", dx, dy);
                proto.x = constrain(x+dx, 0, MATRIX_X-1);
                proto.y = constrain(y+dy, 0, MATRIX_Y-1);

                if (!walls_are_blocking || !in(&proto, sandbox)) {
                    x = proto.x;
                    y = proto.y;
                    Log.info("(%d,%d)", x, y);
                    return true;
                }
                i++;
            }
            Log.info(" (x,x) nm");
            
            // we've failed to move; 10% chance of teleportation
            if (P(10)) {
                return jump(spot, sandbox);
            } 
            return false;
        }


        void seek(Dot* food[], Dot* sandbox[]) {
            // if any adjacent but not occupied, step there
            //Dot* candidate = adjacent_open(food, sandbox);
            if (step_adjacent_open(food, sandbox)) {
                // Serial.println("found open food, doing that");
            } else {
                int closeish_id = pick_closeish_open(food, sandbox);
                if (closeish_id != -1) {
                    Log.info("(%d,%d) -> a closeish open @ (%d,%d)", 
                        x, y, food[closeish_id]->x, food[closeish_id]->y);
                    move_toward(food[closeish_id], sandbox);
                } else {
                    wander(sandbox);
                }
            }
        }
        
        
        virtual void run(Dot* food[], Dot* sandbox[]) {
            // 99% chance of staying on food
            if (in(this, food) && P(99)) {
                // I'm on food
                // do nothing
            } else {
                seek(food, sandbox);
            }
        }
        
        
        // walk to a square with no adjacent dots.  true on success
        bool avoid(Dot* dots[]) {
            int i = 0;
            while (i < 10) {
                int xprime = constrain(x+random(-1, 2), 0, MATRIX_X-1);
                int yprime = constrain(y+random(-1, 2), 0, MATRIX_Y-1);
                Dot candidate = Dot(xprime, yprime, BLACK);
                if (!any_adjacent(&candidate, dots)) { 
                    x = xprime;
                    y = yprime;
                    return true;
                }
                i++;
            }
            return false;
        }
};



#define Q_RESTING  (Adafruit_NeoPixel::Color(48, 12, 63))
#define Q_BIRTHING (Adafruit_NeoPixel::Color(48*4, 12*4, 63*4))
#define Q_EATING   (Adafruit_NeoPixel::Color(64, 64, 255))


class Queen : public Ant {
    private:
        SimpleTimer* birth_control;
        

    public:
        Queen() : Ant() {
            color = Q_RESTING;
            active = true;
            birth_control = new SimpleTimer(1000);
        }


        void eat_one(Dot* sandbox[]) {
            color = Q_EATING;
            // Serial.println("eating one!");
            int cursor = next(0, sandbox); // skip me
            while (cursor != -1) {
                if (sandbox[cursor]->adjacent(this)) {
                    // Serial.printf("maybe Deactivating ant #%d\n", cursor);
                    deactivate(cursor, sandbox);
                    // delay(100);
                    return;
                }
                cursor = next(cursor, sandbox);
            }
        }
        
        
        void birth_one(Dot* sandbox[]) {
            color = Q_BIRTHING;
            // Serial.println("Birthing one!");
            Ant* bb = (Ant*)activate(sandbox);
            bb->x = x;
            bb->y = y;
        }


        void rest(Dot* sandbox[]) {
            color = Q_RESTING;
            if (P(75)) {
                avoid(sandbox);
            } else {
                wander(sandbox);
            }
        }

        
        void run(Dot* food[], Dot* sandbox[]) override {
            if (len(food) < len(sandbox) - 1 && P(50)) {
                eat_one(sandbox);
            } else if (len(food) > len(sandbox) -1 && birth_control->isExpired()) {
                birth_one(sandbox);
            } else {
                rest(sandbox);
            }

        }
};


#define RESTING  0
#define FETCHING 1
#define BUILDING 2
#define SPAZZING 3
#define CLEANING 4
#define DUMPING  5

#define FRAGGLE_SPEED 350 // ms per action or step


/*
 * pick_closeish_open(plan, sandbox) => returns index of plan without a brick
 *   -1: plan is complete
 *
 * pick_closeish_open(sandbox, plan) => returns index of a brick not in the plan
 *   -1: no mess
 */

  class Fraggle: public Ant {
      protected:
          Dot* target;
          uint8_t state, prev_state;
          int8_t patience;
          bool walls_are_blocking;
          SimpleTimer *pacer;
      

          // adjust candidate to be a cell adjacent to me and not otherwise
          // represented in sandbox.  returns True if such exists
          virtual bool randomize(Dot* candidate, Dot* sandbox[]) override {
              int i = 0;
              while (i < 16) {
                  candidate->x = constrain(x+random(-1, 2), 0, MATRIX_X-1);
                  candidate->y = constrain(y+random(-1, 2), 0, MATRIX_Y-2);
                   if (!in(candidate, sandbox)) { 
                      return true;
                   }
                   i++;
              }
              return false;
          }

      
          // returns 1 brick, or nullptr
          Dot* find_loose_brick(Dot* plan[], Dot* sandbox[]) {
              Log.info("finding loose brick");
              // TODO: return an adjacent if available
              int i = pick_closeish_open(sandbox, plan, RED);
              if (i == -1) {
                 i = pick_closeish_open(sandbox, plan, DARKRED);
              }
              if (i != -1) {
                  Log.info("picked %d (%d,%d), 0x%06lx", i,
                           sandbox[i]->x, sandbox[i]->y, sandbox[i]->color);
                  return sandbox[i];
              }
              return nullptr;
          }
          
          
          bool is_brick(Dot* target) {
              return (target 
                      && ((target->get_color() == RED) 
                          || (target->get_color() == DARKRED)));
          }
          
          
          // returns 1 brick, or nullptr
          Dot* adjacent_loose_brick(Dot* plan[], Dot* sandbox[]) {
              #ifdef PRINTF_DEBUGGER
                  Serial.printf("Scanning for a loose brick near (%d,%d)\n", x, y);
              #endif
              Dot candidate = Dot(x, y, color);
              for (int i = 0; i < 10; i++) {
                  candidate.x = constrain(x+random(-1, 2), 0, MATRIX_X-1);
                  candidate.y = constrain(y+random(-1, 2), 0, MATRIX_Y-1);
                  #ifdef PRINTF_DEBUGGER
                      Serial.printf("candidate: (%d,%d)\n", candidate.x, candidate.y);
                  #endif
                  if (!in(&candidate, plan)) {
                      #ifdef PRINTF_DEBUGGER
                          Serial.println("not in plan...");
                      #endif
                      Dot* possible_brick = in(&candidate, sandbox);
                      #ifdef PRINTF_DEBUGGER
                          if (possible_brick) {
                              Serial.printf("possible match: (%d,%d)\n", possible_brick->x, possible_brick->y);
                          }
                      #endif
                      if (is_brick(possible_brick)) {
                          #ifdef PRINTF_DEBUGGER
                              Serial.println("hit!!");
                          #endif
                          return possible_brick;
                      }
                  }
              }
              return nullptr;
          }
          
          
          // returns a plan location, or nullptr
          Dot* find_open_plan(Dot* plan[], Dot* sandbox[]) {
              #ifdef PRINTF_DEBUGGER
                  Serial.printf("Fraggle(%d,%d) finding open plan spot\n", x, y);
              #endif
              // TODO: prefer adjacent
              int i = pick_closeish_open(plan, sandbox);
              #ifdef PRINTF_DEBUGGER
                  Serial.printf("found %d: (%d,%d)\n", i, plan[i]->x, plan[i]->y);
              #endif
              if (i != -1) {
                  return plan[i];
              }
              return nullptr;
          }

          
          
          void place_brick(Dot* target, color_t color, Dot* sandbox[]) {
              Dot* brick = activate(sandbox);
              brick->set_color(color);
              brick->x = target->x;
              brick->y = target->y;
          }


          void pick_up(Dot* brick, Dot* sandbox[]) {
              deactivate(brick, sandbox);
          }


          void not_stuck() {
              patience = 10;
          }
          
          
          void maybe_stuck(Dot* sandbox[]) {
              patience --;
              Log.info("patience: %d\n", patience);
              if (patience == 5) {
                  if (P(10) && jump(target, sandbox)) {
                      return;
                  } else {
                      // force a new target
                      target = nullptr;
                  }
              } else if (patience <= 0) {
                  prev_state = state;
                  state = SPAZZING;
                  // target = new Dot();
                  // target->x = x;
                  // target->y = y;
              }
          }
          
          
          
      public:
          Fraggle(): Ant(), state(RESTING), walls_are_blocking(true) {
              active = true;
              patience = 10;
              // Particle.function("fraggle_walls", &Fraggle::toggle_walls, this);
              pacer = new SimpleTimer(FRAGGLE_SPEED);
              target = nullptr;
          } // Fraggle()


          void start_fetching(Dot* plan[], Dot* sandbox[]) {
              target = find_loose_brick(plan, sandbox);
              if (target) {
                  Log.info("starting fetch; target=(%d,%d)", 
                           target->x, target->y);
                  state = FETCHING;
              } else {
                  Log.info("didn't find a target; resting");
                  state = RESTING;
              }
          } // start_fetching(plan, sandbox)


          // I seek an open brick (not in the plan)
          void fetch(Dot* plan[], Dot* sandbox[]) {
              color = BLUE;
              Log.info("fetching");
              // an invalid target is nullptr, is in the plan, is not in the sandbox, or is not a brick
              if (!target) {
                  Log.info("no target");
              }

              Log.info("checking target (%d,%d)", target->x, target->y);

              if (in(target, plan)) {
                  Log.info("target in plan");
              }

              if (!in(target, sandbox)) {
                  Log.info("target not in sandbox");
              }

              if (!is_brick(target)) {
                  Log.info("target is not a brick");
              }

              if (!target 
                  || in(target, plan) 
                  || !in(target, sandbox) 
                  || !is_brick(target)) {
                  Log.info("no target");
                  state = RESTING;
                  return;
              }
              Log.info("fetching brick(%d,%d): d=%4.1f", 
                  target->x, target->y, distance_to(target));
              if (adjacent(target) && is_brick(target)) { 
                  // found it
                  Log.info("picking up");
                  pick_up(target, sandbox);
                  start_building(plan, sandbox);
                  return;
                  // TODO start_building()
                  target = nullptr;
                  state = BUILDING;
                  return;
              }
              Log.info("moving toward that brick");
              if (move_toward(target, sandbox, walls_are_blocking)) {
                  Log.info("success: (%d,%d)\n", x, y);
              } else {
                  Log.info("I might be stuck?");
                  target = adjacent_loose_brick(plan, sandbox);
                  if (target) {
                      Log.info("found a brick(%d,%d)\n", target->x, target->y);
                      not_stuck();
                  } else {
                      maybe_stuck(sandbox);
                  }
              }
          } // fetch(plan, sandbox)
          

          void start_building(Dot* plan[], Dot* sandbox[]) {
              target = find_open_plan(plan, sandbox);
              if (target) {
                  Log.info("starting build with target=(%d,%d)", 
                           target->x, target->y);
                  state = BUILDING;
              } else {
                  state = RESTING;
              }
          } // start_building(plan, sandbox)


          /*
           * I have a brick, and need to put it on a plan spot
           * if no plan spots, put it in the bin
           * 
           * target: the intended location for my brick
           */
          void build(Dot* plan[], Dot* sandbox[]) {
              Log.info("building");
              if (! target) {
                  Log.info("no target");
              }

              if (in(target, sandbox) ) {
                  Log.info("target already in sandbox");
              }

              if (! target || in(target, sandbox)) {
                  Log.info("no target, or in sandbox; dump");
                  start_dumping(plan, sandbox);
                  return;
              }
              Log.info("targetting (%d,%d)", target->x, target->y);
              if (adjacent(target)) {
                  // found it
                  Log.info("placing brick at (%d,%d)", target->x, target->y);
                  place_brick(target, RED, sandbox);
                  state = RESTING;
                  target = nullptr;
                  return;
              }
              if (!move_toward(target, sandbox, walls_are_blocking)) {
                  maybe_stuck(sandbox);
                  target = find_open_plan(plan, sandbox);
              } else {
                  not_stuck();
              }
          } // build(plan, sandbox)
          

          void start_cleaning(Dot* plan[], Dot* sandbox[]) {
              target = find_loose_brick(plan, sandbox);
              state = CLEANING;
          } // start_cleaning(plan, sandbox)


          void clean(Dot* plan[], Dot* sandbox[]) {
              Log.info("Clean()");
              // TODO: validate target
              Dot* target = find_loose_brick(plan, sandbox);
              if (target) {
                  Log.info("maybe cleaning (%d,%d)", target->x, target->y);
                  if (adjacent(target) && is_brick(target)) { 
                      // found it
                      Log.info("picking up");
                      pick_up(target, sandbox);
                      target = nullptr;
                      state = DUMPING;
                      return;
                  } else {
                      if (move_toward(target, sandbox, walls_are_blocking)) {
                          not_stuck();
                      } else {
                          maybe_stuck(sandbox);
                      }
                  }
              } else {
                  Log.info("nothing to clean; resting");
                  state = RESTING;
              }
          } // void clean(plan, sandbox)


          Dot* first_available(Dot* bin[], Dot* sandbox[]) {
              for (int i = 0; i < MAX_DOTS; i++) {
                  if (bin[i]->active && !in(bin[i], sandbox)) {
                      return bin[i];
                  }
              }
              return nullptr;
          } // Dot* first_available(bin, sandbox)

          
          void start_dumping(Dot* bin[], Dot* sandbox[]) {
              target = first_available(bin, sandbox);
              state = DUMPING;
          } // start_dumping(bin, sandbox)


          void dump(Dot* bin[], Dot* sandbox[]) {
              Log.info("Dump()");

              // target invalid if nullptr, not a bin location, or occupied
              if (! target || !in(target, bin) || in(target, sandbox)) {
                  target = first_available(bin, sandbox);
              }
              
              Log.info("dump target: (%d,%d)", target->x, target->y);
              if (adjacent(target) && !in(target, sandbox)) {
                  Log.info("dumping here!");
                  Dot* rubbish = activate(sandbox);
                  rubbish->x = target->x;
                  rubbish->y = target->y;
                  rubbish->color = DARKRED;
                  target = nullptr;
                  state = RESTING;
                  return;
              }
              if (move_toward(target, sandbox, walls_are_blocking)) {
                  not_stuck();
              } else {
                  maybe_stuck(sandbox);
              }
          }
          
          
          void spaz(Dot* sandbox[]) {
              // if (distance_to(target) >= 3) {
              if (patience >= 10) {
                  // done spazzing
                  state = prev_state;
                  // delete target;
              } else {
                  if (P(50)) {
                      ++patience;
                  }
                  wander(sandbox);
              }
          }
          
          
          void rest(Dot* plan[], Dot* sandbox[]) {
              state = RESTING;
              //TODO target = nullptr;
              
              Log.info("resting.  Picking open plan, sandbox to check for open plan spots");
              int i = pick_closeish_open(plan, sandbox);
              if (i != -1) {
                  // plan missing bricks; fetch & build
                  Log.info("found %d; fetching", i);
                  start_fetching(plan, sandbox);
                  return;
                  //TODO target = nullptr;
                  state = FETCHING;
              } else {
                Log.info("looking for RED bricks in sandbox, not plan");
                if ((i = pick_closeish_open(sandbox, plan, RED)) != -1) {
                    // if bricks not on plan, clean
                    Log.info("found %d; cleaning", i);
                    state = CLEANING;            
                    //TODO target = nullptr;
                } else if (P(10)) {
                    wander(sandbox);
                } else {
                // Serial.println("Avoiding");
                    avoid(sandbox);
                }
            }
        }


        // the bin is ONLY a set of locations, in priority, order, for dumping
        void run(Dot* plan[], Dot* sandbox[], Dot* bin[]) {
            if (! pacer->isExpired()) {
                return;
            }
            Log.info("Fraggle[%d](%d, %d):%d\n", id, x, y, state);
            switch (state) {
                case SPAZZING:
                    color = YELLOW;
                    spaz(sandbox);
                    break;
                case FETCHING:
                    fetch(plan, sandbox);
                    break;
                case BUILDING:
                    color = GREEN;
                    build(plan, sandbox);
                    break;
                case CLEANING:
                    color = BLUE;
                    clean(plan, sandbox);
                    break;
                case DUMPING:
                    color = MAGENTA;
                    dump(bin, sandbox);
                    break;
                case RESTING: 
                default:
                    color = WHITE;
                    rest(plan, sandbox);
            }
        }
};

#endif
