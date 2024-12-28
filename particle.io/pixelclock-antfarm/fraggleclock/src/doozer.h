#ifndef _DOOZER_H_
#define _DOOZER_H_

#include "dot.h"
#include "ant.h"
#include "turtle.h"
#include "list.h"

#define WALK_SPEED  500 // ms per step
#define REST_SPEED 1000 // ms per step


struct coord_struct { 
  byte x;
  byte y;
};

#if (ASPECT_RATIO == SQUARE) 
  // horizontal, two rows
  struct coord_struct
     TOP_LEFT = { .x = 0, .y = MATRIX_Y-2 },
     BOTTOM_RIGHT = { .x = MATRIX_X - 1, .y = MATRIX_Y - 1 };
#else
  // vertical, two rows
  struct coord_struct
     TOP_LEFT = { .x = 0, .y = 0 },
     BOTTOM_RIGHT = { .x = MATRIX_X - 1, .y = MATRIX_Y - 1 };
#endif


/*
 *  A Doozer is a hard-workin Turtle
 *  A Fraggle is a stupid doozer
 * 
 *  if track exists in sandbox but not plan
 *      state == CLEAN
 *      clean the closest one
 *  if a missing spot exists (plan without tracks)
 *      state == BUILD
 *      go there & make a track
 *      
 *  No restriction on where-to-walk
 */

class Doozer: public Turtle {
    private:
        Dot* target;
        SimpleTimer *rest_timer;
        byte min_iq;


        void check_progress() {
            static byte prev_state = RESTING;
            static float distance = 0;
            
            // definitely reset if resting or new state
            if (RESTING || (state != prev_state)) {
                iq = min_iq;
                distance = 0;
                if (target && (state != RESTING)) {
                    distance = distance_to(target);
                }
                prev_state = state;
                return;
            }

            // only smarten on the 8s
            if (Time.minute() % 10 != 8) {
                return;
            }

            float new_distance = distance_to(target);
            if (new_distance >= distance_to(target)
                && (iq < 25)) {
                iq = min(iq+1, 25);
                Log.info("smartening: iq=%d", iq);
            }
            distance = new_distance;
        } // check_progress()


        /* 
         * a loose brick is NOT in the plan, but IS in the sandbox
         *
         * special case: the lower two rows are "bins", last resort
         */
        Dot* find_loose_brick(Dot* plan[], Dot* sandbox[]) {
            int cursor;
            Dot* proxy;

            Log.info("finding a loose brick");

            // stg 1: search the non-bin
            for (cursor = first(sandbox); 
                 cursor != -1; 
                 cursor = next(cursor, sandbox)) {
                proxy = sandbox[cursor];
                if (is_brick(proxy)) {
                    Log.trace("is brick: (%d,%d)", proxy->x, proxy->y);
                }
                if (! in(proxy, plan)) {
                    Log.trace("not in plan either");
                }
                if (proxy->y < MATRIX_Y - 3 
                    && is_brick(proxy)
                    && !in(proxy, plan)) {
                    Log.trace("Found #1 (%d,%d)", proxy->x, proxy->y);
                    return proxy;
                }
            }

            // print_list(sandbox);

            for (cursor = first(sandbox); 
                 cursor != -1; 
                 cursor = next(cursor, sandbox)) {
                proxy = sandbox[cursor];
                if (is_brick(proxy)
                    && !in(proxy, plan)) {
                    Log.trace("Found #1 (%d,%d)", proxy->x, proxy->y);
                    return proxy;
                }
            }

            // if all else fails... fail
            Log.info("brick not found");
            return nullptr;
        } // Dot* find_loose_brick(Dot* plan[], Dot* sandbox[])


        // returns a plan location, or nullptr
        Dot* find_open_plan(Dot* plan[], Dot* sandbox[]) {
          Log.trace("Doozer(%d,%d) finding open plan spot", x, y);
          // TODO: prefer adjacent
          int i = pick_closeish_open(plan, sandbox);
          if (i != -1) {
            Log.trace("found %d: (%d,%d)", i, plan[i]->x, plan[i]->y);
            return plan[i];
          }
          Log.trace("did not find :(");
          return nullptr;
        } // Dot* find_open_plan(plan, sandbox)


        void fetch(Dot* plan[], Dot* sandbox[]) {
            Log.trace("fetch()");
            Log.trace("target? %c", target != nullptr ? 'y' : 'n');
            if(target) {
              Log.trace("target @ (%d,%d)", target->x, target->y);
              Log.trace("in(target, plan) ? %c", in(target, plan) ? 'y' : 'n');
              Log.trace("!in(target, sandbox) ? %c", !in(target, sandbox) ? 'y' : 'n');
              Log.trace("!is_brick(target) ? %c", !is_brick(target) ? 'y' : 'n');
            }
        
            if (! target 
                || in(target, plan) 
                || !in(target, sandbox) 
                || !is_brick(target)) {
                Log.trace("fetch() -> find_loose_brick()");

                target = find_loose_brick(plan, sandbox);
            }

            if (! target) {
                Log.trace("fetch(): no target?");

            }

            if (! target) {
                Log.trace("fetch(): STILL no target, resting");
                // nothing to fetch; rest
                state = RESTING;
                return;
            }

            Log.info("Fetching: target @ (%d,%d)", target->x, target->y);

            if ((equals(target) || adjacent(target)) 
                && is_brick(target)) {
                Log.info("Picking up (%d,%d)", target->x, target->y);
                pick_up(target, sandbox);
                // print_sandbox(sandbox);
                target = nullptr;
                state = BUILDING;
                return;
            }

            // step_toward(target);
            if (!move_toward(target, sandbox)) {
                wander(sandbox);
            }
            return;
        }  // void fetch(Dot* plan[], Dot* sandbox[])


        void build(Dot* plan[], Dot* sandbox[]) {
            if (!target || in(target, sandbox)) {
                target = find_open_plan(plan, sandbox);
            }

            if (!target) {
                // nowhere to go but I'm holding
                state = DUMPING;
                return;
            }

            if (adjacent(target)) {
                place_brick(target, TIME_COLOR, sandbox);
                target = nullptr;
                state = RESTING;
                return;
            }

            // step_toward(target);
            if (!move_toward(target, sandbox)) {
                wander(sandbox);
            }
            return;
        }


        void clean(Dot* plan[], Dot* sandbox[]) {
            Log.info("cleaning: (%d,%d)", x, y);

            if (!target 
                || in(target, plan) 
                || !in(target, sandbox)) {
                target = find_loose_brick(plan, sandbox);
            }

            if (!target) {
                Log.info("no target found; resting");
                state = RESTING;
                return;
            }

            /*
            Log.info("want to clean (%d,%d) 0x%06lx", 
                     target->x, target->y, target->color);
            Log.info("adjacent? %c", adjacent(target) ? 'y':'n');
            Log.info("brick? %c", is_brick(target) ? 'y':'n');
            Log.info("color 0x%06lx? %c", TIME_COLOR, 
                           target->color == TIME_COLOR ? 'y':'n');
            Log.info("color 0x%06lx? %c", UNUSED_BRICK_COLOR, 
                     target->color == UNUSED_BRICK_COLOR ? 'y':'n');
            */

            if ((adjacent(target) || equals(target)) && is_brick(target)) {
                Log.info("adjacent; dumping");
                pick_up(target, sandbox);
                target = nullptr;
                state = DUMPING;
                return;
            }

            Log.info("stepping (%d,%d) -> (%d,%d)", 
                      x, y, target->x, target->y);
            // step_toward(target);
            if (!move_toward(target, sandbox)) {
                wander(sandbox);
            }
            return;
        } // void clean(Dot* plan[], Dot* sandbox)


        // return the best bin location, for dumping
        // scan from bottom to top, right to left
        Dot* best_bin_location(Dot* sandbox[]) {
          Dot* location;
         
          // Log.info(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>");
          // Log.info("TL: (%d,%d), BR: (%d,%d)", 
          //          TOP_LEFT.x, TOP_LEFT.y,
          //          BOTTOM_RIGHT.x, BOTTOM_RIGHT.y);
          for (int y = BOTTOM_RIGHT.y; y >= TOP_LEFT.y; y--) {
            for (int x = BOTTOM_RIGHT.x; x >= TOP_LEFT.x; x--) {
              // Log.info("possible bin location: (%d,%d)", x, y);
              // Log.info("x %d == TL.x %d?", x, TOP_LEFT.x);
              // Log.info("x %d == BR.x %d?", x, BOTTOM_RIGHT.x);
              #if (ASPECT_RATIO == WIDESCREEN) 
                if ((x != TOP_LEFT.x) && (x != BOTTOM_RIGHT.x)) {
                  continue;
                }
              #endif
              // Log.info("possible bin location: (%d,%d)", x, y);
              if (!in(x, y, sandbox)) {
                location = activate(sandbox);
                location->x = x;
                location->y = y;
                location->color = BLACK;
                // Log.info("best bin location: (%d,%d)", x, y);
                // Particle.publish("doozer", 
                //    String::format("best bin location: (%d,%d)", x, y));
                return location;
              }
            }
          }
          // FALLTHROUGH
          return nullptr;
        } // Dot* best_bin_location(Dot* sandbox[])


        void dump(Dot* sandbox[]) {
            Log.info("dumping...");
            if (! target) {
                Log.info("before best_bin_location");
                // print_sandbox(sandbox);
                target = best_bin_location(sandbox);
                Log.info("after best_bin_location");
                // print_sandbox(sandbox);
            }

            if (! target) {
                Log.info("no target, resting");
                state = RESTING;
                return;
            }

            Log.info("dump target (%d,%d)", target->x, target->y);

            if (equals(target) || adjacent(target)) {
                target->set_color(UNUSED_BRICK_COLOR);
                target = nullptr;
                state = RESTING;
                return;
            }

            // step_toward(target);
            if (!move_toward(target, sandbox)) {
                wander(sandbox);
            }
            // check_progress();
            return;
        } // void dump(Dot* sandbox[])


        // if needed, sort the bin
        bool maybe_ocd(Dot* sandbox[]) {
          Dot* ocd_target;
          // scan L -> R
          int py = MATRIX_Y - 2;
          for (int px = 0; px <= MATRIX_X-2; px++) {
              if ((ocd_target = find(x, y, sandbox))
                  && !in(px+1, py, sandbox)) {
                  target = ocd_target;
                  state = CLEANING;
                  return true;
              }
          }

          return false;
        } // bool maybe_ocd(Dot* sandbox[])


        void rest(Dot* plan[], Dot* sandbox[]) {
            int i = pick_closeish_open(plan, sandbox);
            if (i != -1) {
                // plan missing bricks; fetch & build
                Log.info("found %d (%d,%d); fetching", i, 
                    plan[i]->x, plan[i]->y);
                state = FETCHING;
                return;
            }

            i = pick_closeish_open(sandbox, plan, TIME_COLOR);
            if (i != -1) {
                Log.info("found %d not in plan; cleaning\n", i);
                state = CLEANING;
                return;
            }

            /* TODO: this
            if (P(25) && maybe_ocd(sandbox)) {
                return;
            }
            */

            if (rest_timer->isExpired()) {
                if (! avoid(sandbox) || P(10)) {
                    Log.info("wandering: (%d,%d)", x, y);
                    wander(sandbox);
                }
            }
            if ((millis() / 1000) % 2) {
                // "tick"
                color = color / 2;
            }
        } // void rest(Dot* plan[], Dot* sandbox[])


    public:
        Doozer() : Turtle() {
            color = MIDWHITE;
            state = RESTING;
            delay(WALK_SPEED/2);
            Log.info("creating doozer at %lu", millis());
            step_timer->setInterval(WALK_SPEED);
            rest_timer = new SimpleTimer(REST_SPEED);
            iq = min_iq;
            target = nullptr;
            x = id % 4;
            y = id % 4;
        }; // constructor


        void set_iq(int new_iq) {
            if (new_iq > 0) {
                iq = min_iq = new_iq;
            }
        } // set_iq(iq)


        void run(Dot* plan[], Dot* sandbox[]) override {
            static unsigned long last_rested = 0;
            // delay(1000);
            if (! step_timer->isExpired()) {
                Log.trace("doozer run() fini");
                return;
            }
            Log.info("Doozer[%d](%d, %d):%d @ %lu", id, x, y, state, millis());

            // delay(1000);
            switch (state) {
              case FETCHING:
                // color = GREEN;
                Log.trace("fetching");
                fetch(plan, sandbox);
                break;
              case BUILDING:
                // color = MAGENTA;
                build(plan, sandbox);
                break;
              case CLEANING:
                // color = MIDGREEN;
                clean(plan, sandbox);
                break;
              case DUMPING:
                // color = MAGENTA;
                dump(sandbox);
                break;
              case RESTING:
              default:
                // color = MIDWHITE;
                rest(plan, sandbox);
                last_rested = millis();
            }
            color = SPRITE_COLOR;

            // if at least 10 minutes since last rest, bump IQ
            if (millis() - last_rested > 10*60*1000) {
                iq = 25;
                color = WHITE;
            } else {
                iq = 0;
            }
        } // run(plan, sandbox)
};


#define NUMBER_OF_DOOZERS 3


void maybe_check_brick_pile(Dot* sandbox[]) {
    // Log.trace("checking brick pile...");
    // always add to bottom-right, if needed
    Dot proxy = Dot(BOTTOM_RIGHT.x, BOTTOM_RIGHT.y, BRICK_COLOR);
    if (!in(&proxy, sandbox)) {
        Log.trace("adding one @ (%d,%d)", proxy.x, proxy.y);
        // print_list(sandbox);
        Dot* brick = activate(sandbox);
        // Serial.printf("touching brick (%d,%d)\n", brick->x, brick->y);
        brick->set_color(UNUSED_BRICK_COLOR);
        brick->x = proxy.x;
        brick->y = proxy.y;
        // print_list(sandbox);
    }

    #if (ASPECT_RATIO == SQUARE)
      // scan the bin top row; remove any bricks
      proxy.y = TOP_LEFT.y;
      for (int x = TOP_LEFT.x; x <= BOTTOM_RIGHT.x; x++) {
        proxy.x = x;
        Dot* brick;
        if ((brick = in(&proxy, sandbox)) &&
            (brick->get_color() == UNUSED_BRICK_COLOR)) {
            Log.trace("removing one");
            deactivate(brick, sandbox);
            return;
        }
      }
    #else 
      // scan the top bit on both sides; remove any bricks
      for (int y = TOP_LEFT.y; y <= 2; y++) {
        Dot* brick;
        proxy.x = TOP_LEFT.x;
        proxy.y = y;
        // Log.trace("widescreen: scanning (%d,%d)", proxy.x, proxy.y);
        if ((brick = in(&proxy, sandbox)) &&
            (brick->get_color() == UNUSED_BRICK_COLOR)) {
            Log.trace("removing one");
            deactivate(brick, sandbox);
            return;
        }
        proxy.x = BOTTOM_RIGHT.x;
        proxy.y = y;
        // Log.trace("widescreen: scanning (%d,%d)", proxy.x, proxy.y);
        if ((brick = in(&proxy, sandbox)) &&
            (brick->get_color() == UNUSED_BRICK_COLOR)) {
            Log.trace("removing one");
            deactivate(brick, sandbox);
            return;
        }
      }
    #endif

    // print_sandbox(sandbox);
} // maybe_check_brick_pile(sandbox)


/*
 * FRAGGLES
 * ... actually just simpler Doozers
 *
 */

void update_doozer_layout(Layout* layout, Dot* sandbox[]) {
  if (layout->show_weather) {
    TOP_LEFT.x = 1;
    BOTTOM_RIGHT.x = MATRIX_X - 2;
  } else {
    TOP_LEFT.x = 0;
    BOTTOM_RIGHT.x = MATRIX_X - 1;
  }

  if (layout->show_pinger) {
     #if (ASPECT_RATIO == SQUARE) 
       TOP_LEFT.y = MATRIX_Y - 3,
     #endif
     BOTTOM_RIGHT.y = MATRIX_Y - 2;
  } else {
     #if (ASPECT_RATIO == SQUARE) 
       TOP_LEFT.y = MATRIX_Y - 2,
     #endif
     BOTTOM_RIGHT.y = MATRIX_Y - 1;
  }

  Particle.publish("doozer", 
    String::format("top left: (%d,%d); bottom right: (%d,%d)",
                   TOP_LEFT.x, TOP_LEFT.y, 
                   BOTTOM_RIGHT.x, BOTTOM_RIGHT.y));
} // update_doozer_layout(layout, sandbox)


#define NUMBER_OF_FRAGGLES 2

void setup_fraggles(Dot* sandbox[]) {
    Log.info("Making fraggles");
    for (int i = 0; i < NUMBER_OF_FRAGGLES; i++) {
        Log.info("making %d at %lu", i, millis());
        sandbox[i] = new Doozer();
    }
    for (int i = NUMBER_OF_FRAGGLES; i < MAX_DOTS; i++) {
        sandbox[i] = new Dot();
    }
} // make_fraggles()


void loop_fraggles(Dot* food[], Dot* sandbox[]) {
    static SimpleTimer sec(1000);

    if (sec.isExpired()) {
        maybe_check_brick_pile(sandbox);
    }

    for (int i = 0; i < NUMBER_OF_FRAGGLES; i++) {
        Doozer* doozer = (Doozer*)sandbox[i];
        doozer->run(food, sandbox);
    }
} // loop_fraggles()


void setup_doozers(Dot* sandbox[]) {
    Log.trace("Making doozers");
    Doozer* d;
    // make NUMBER_OF_DOOZERS Doozer()s
    for (int i = 0; i < NUMBER_OF_DOOZERS; i++) {
        sandbox[i] = new Doozer();
        d = (Doozer *)sandbox[i];
    }
    // first one is smarter
    d = (Doozer *)sandbox[0];
    d->set_iq(25);

    // make Dots for the rest
    for (int i = NUMBER_OF_DOOZERS; i < MAX_DOTS; i++) {
        sandbox[i] = new Dot();
    }
} // setup_doozers(sandbox)


void loop_doozers(Dot* food[], Dot* sandbox[]) {
    static SimpleTimer second(1000);
    if (second.isExpired()) {
        // maybe_adjust_one_brick();
        maybe_check_brick_pile(sandbox);
    }

    for (int i = 0; i < NUMBER_OF_DOOZERS; i++) {
        Doozer* doozer = (Doozer*)sandbox[i];
        // Log.trace("doozer run()");
        doozer->run(food, sandbox);
        // Log.trace("doozer ran()");
    }

    // Log.trace("loop_doozers end");
} // loop_doozers()

#endif
