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
            if (! target 
                || in(target, plan) 
                || !in(target, sandbox) 
                || !is_brick(target)) {
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
                pick_up(target, sandbox);
                target = nullptr;
                state = BUILDING;
                return;
            }

            step_toward(target);
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

            step_toward(target);
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

            Log.info("want to clean (%d,%d) 0x%06x", 
                     target->x, target->y, target->color);
            Log.info("adjacent? %c", adjacent(target) ? 'y':'n');
            Log.info("brick? %c", is_brick(target) ? 'y':'n');
            Log.info("color 0x%06x? %c", RED, target->color == RED ? 'y':'n');
            Log.info("color 0x%06x? %c", DARKRED, target->color == DARKRED ? 'y':'n');

            if ((adjacent(target) || equals(target)) && is_brick(target)) {
                Log.info("adjacent; dumping");
                pick_up(target, sandbox);
                target = nullptr;
                state = DUMPING;
                return;
            }

            Log.info("stepping (%d,%d) -> (%d,%d)", 
                      x, y, target->x, target->y);
            step_toward(target);
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
            if (! target
                || !is_bin(target)
                || in(target, sandbox)) {
                target = best_bin_location(sandbox);
            }

            if (! target) {
                Log.info("no target, resting");
                state = RESTING;
                return;
            }

            Log.info("dump target (%d,%d)", target->x, target->y);

            if ((equals(target) || adjacent(target))
                && !is_brick(target)) {
                target->color = DARKRED;
                target = nullptr;
                state = RESTING;
                return;
            }

            step_toward(target);
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
            step_timer->setInterval(WALK_SPEED);
            rest_timer = new SimpleTimer(REST_SPEED);
        }; // constructor


        void run(Dot* plan[], Dot* sandbox[]) override {
            if (! step_timer->isExpired()) {
                return;
            }
            Log.info("Doozer[%d](%d, %d):%d", id, x, y, state);
            switch (state) {
              case FETCHING:
                color = GREEN;
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
            }
        }
};

#endif
