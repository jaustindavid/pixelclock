#ifndef _DOOZER_H_
#define _DOOZER_H_

#include "dot.h"
#include "ant.h"
#include "turtle.h"
#include "list.h"

#define WALK_SPEED  500 // ms per step
#define REST_SPEED 1000 // ms per step


/*
    A Doozer is a hard-workin Turtle
    A Fraggle is a stupid doozer
   
    if track exists in sandbox but not plan
        state == CLEAN
        clean the closest one
    if a missing spot exists (plan without tracks)
        state == BUILD
        go there & make a track
        
    No restriction on where-to-walk
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

            // stg 2: search the bin L->R, T->B
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


        void fetch(Dot* plan[], Dot* sandbox[]) {
            Log.trace("fetch()");
            Log.trace("target? %c", target == nullptr ? 'y' : 'n');
            if(target) {
              Log.trace("in(target, plan) ? %c", in(target, plan) ? 'y' : 'n');
              Log.trace("!in(target, sandbox) ? %c", !in(target, sandbox) ? 'y' : 'n');
              Log.trace("!is_brick(target) ? %c", !is_brick(target) ? 'y' : 'n');
            }
            Log.trace("boop");
            if (! target 
                || in(target, plan) 
                || !in(target, sandbox) 
                || !is_brick(target)) {
                Log.trace("fetch() -> find_loose_brick()");
                Log.trace("boop");

                target = find_loose_brick(plan, sandbox);
            }

            if (! target) {
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
                place_brick(target, RED, sandbox);
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

            Log.info("want to clean (%d,%d) 0x%06lx", 
                     target->x, target->y, target->color);
            Log.info("adjacent? %c", adjacent(target) ? 'y':'n');
            Log.info("brick? %c", is_brick(target) ? 'y':'n');
            Log.info("color 0x%06lx? %c", RED, target->color == RED ? 'y':'n');
            Log.info("color 0x%06lx? %c", DARKRED, target->color == DARKRED ? 'y':'n');

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


        bool is_bin(Dot* target) {
          return (target->y >= (MATRIX_Y - 3))
              && (target->y <= (MATRIX_Y - 2));
        }


        // return the best bin location
        Dot* best_bin_location(Dot* sandbox[]) {
            Dot* location;
            for (int y = MATRIX_Y - 2; y >= MATRIX_Y - 3; y--) {
                for (int x = MATRIX_X - 1; x >= 0; x --) {
                    if (!in(x, y, sandbox)) {
                        location = activate(sandbox);
                        location->x = x;
                        location->y = y;
                        location->color = BLACK;
                        return location;
                    }
                }
            }

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
                target->set_color(DARKRED);
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
                Log.info("found %d; fetching\n", i);
                state = FETCHING;
                return;
            }

            i = pick_closeish_open(sandbox, plan, RED);
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
        }; // constructor


        void set_iq(int new_iq) {
            if (new_iq > 0) {
                iq = min_iq = new_iq;
            }
        }


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
                color = GREEN;
                Log.trace("fetching");
                fetch(plan, sandbox);
                break;
              case BUILDING:
                color = MAGENTA;
                build(plan, sandbox);
                break;
              case CLEANING:
                color = MIDGREEN;
                clean(plan, sandbox);
                break;
              case DUMPING:
                color = MAGENTA;
                dump(sandbox);
                break;
              case RESTING:
              default:
                color = MIDWHITE;
                rest(plan, sandbox);
                last_rested = millis();
            }

            // if at least 10 minutes since last rest, bump IQ
            if (millis() - last_rested > 10*60*1000) {
                iq = 25;
            } else {
                iq = 0;
            }
        } // run(plan, sandbox)
};


#define NUMBER_OF_DOOZERS 3

void maybe_check_brick_pile(Dot* sandbox[]) {
    Dot proxy = Dot(MATRIX_X-1, MATRIX_Y-2, DARKRED);
    if (!in(&proxy, sandbox)) {
        Log.trace("adding one");
        // print_list(sandbox);
        Dot* brick = activate(sandbox);
        // Serial.printf("touching brick (%d,%d)\n", brick->x, brick->y);
        brick->set_color(DARKRED);
        brick->x = proxy.x;
        brick->y = proxy.y;
        // print_list(sandbox);
    }

    // scan the bin top row; remove any bricks
    proxy.y = MATRIX_Y - 3;
    for (int x = 0; x < MATRIX_X; x++) {
        proxy.x = x;
        Dot* brick;
        if ((brick = in(&proxy, sandbox)) &&
            (brick->get_color() == DARKRED)) {
            // Serial.printf("removing: x=%d\n", x);
            deactivate(brick, sandbox);
        }
    }
} // maybe_check_brick_pile(sandbox)


/*
 * FRAGGLES
 * ... actually just simpler Doozers
 *
 */

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
        // d->setup();
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
        Log.trace("doozer run()");
        doozer->run(food, sandbox);
        Log.trace("doozer ran()");
    }

    Log.trace("loop_doozers end");
} // loop_doozers()

#endif
