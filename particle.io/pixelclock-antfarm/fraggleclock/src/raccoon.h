#pragma once

#include <SimpleTimer.h>
#include "locker.h"
#include "dot.h"
#include "list.h"
#include "ant.h"
#include "turtle.h"


/*
   A Raccoon is... trash panda.

   * pixels not in plan are DIRTY_COLOR (yellow, maybe)
   * pixels in plan are CLEAN_COLOR (green, maybe)
   * a Raccoon will grab a DIRTY pixel, run it to the pool,
   *   wash it (dunk a few times) to turn it CLEAN, then
   *   run it to a spot it belongs

   states:
     CLEANING: finding a DIRTY_COLOR item
     WASHING : taking the item to the pool
     DUNKING : actually flopping the thing a bit
     PLACING : returning a CLEAN_COLOR item
     RESTING : none of the above


     *: change_all(UNUSED, DIRTY_COLOR)
     CLEANING: pick_up(DIRTY_COLOR), state -> WASHING 
     WASHING: go(pool, state -> DUNKING)
     DUNKING: wait(state -> PLACING)
     PLACING: put(CLEAN_COLOR, state -> RESTING)
     RESTING: if_any(DIRTY_COLOR, state -> CLEANING)
              if_any(EMPTY, state -> WASHING)
              wander()

   turtle:
     CLEANING: pick_up(UNUSED), state -> CLEANING
               state->RESTING
     PLACING: put(), state -> PLACING
              state->RESTING
     RESTING: if_any(UNUSED), state->CLEANING
              wander()

   fraggle:
     CLEANING: pick_up(UNUSED) state -> PLACING
     PLACING: put(MISSING) state -> RESTING
              state -> DUMPING
     DUMPING: goto(bottom_row) state -> RESTING
     FETCHING: goto(bottom_row) state -> PLACING
     RESTING: if_any(UNUSED) state -> CLEANING
              if_any(MISSING) state -> FETCHING
 */

// resting 0
// fetching 1
#define PLACING BUILDING // 2
// cleaning 3
#define DUNKING DUMPING // 4
#define WASHING 5
#define WANDERING 6

#if (ASPECT_RATIO == SQUARE)
  #if defined(MEGA)
    #define NRACCOONS 2
  #else
    #define NRACCOONS 1
  #endif
  #define WALK_SPEED  400 // ms per step
  #define REST_SPEED 1000 // ms per step
  #define DUNK_SPEED 1500 // ms per dunking
#else
  #if defined(MEGA)
    #define NRACCOONS 2
  #else
    #define NRACCOONS 1
  #endif
  #define WALK_SPEED  250 // ms per step
  #define REST_SPEED 1000 // ms per step
  #define DUNK_SPEED 1250 // ms per dunking
#endif

#define RACCOON_COLOR SPRITE_COLOR
#define CLEAN_COLOR TIME_COLOR
#define DIRTY_COLOR ALT_COLOR

#define POOL_TARGET (NRACCOONS+1) // sandbox index for a pool node
#define POOL_COLOR (Adafruit_NeoPixel::Color(32, 32, 192))

int TRASH_X = 0, TRASH_Y = MATRIX_Y-1;

#define POOL_LOCK 255
#define TRASH_LOCK 254

class Raccoon: public Turtle {
  private:
    Dot* target;
    SimpleTimer *rest_timer;
    SimpleTimer *dunk_timer;
    uint32_t swish;
    Stopwatch rest_counter, run_counter;


    #if defined(TESTING)
        #define DEBUG_PICKING
    #endif
    // returns a dot of target_color, or -1
    int pick_any(Dot* sandbox[], color_t target_color) {
        int candidates[10];
        int n;
        for (n = 0; n < 10; n++) {
            candidates[n] = -1;
        }

        n = -1;
        for (int i = 0; i < MAX_DOTS; i++) {
            if (sandbox[i]->active 
                && sandbox[i]->color == target_color
                && n < 10) {
                n++;
                candidates[n] = i;
                #if defined(DEBUG_PICKING)
                    Log.info("candidate[%d] = %d", n, i);
                #endif
            }
        }
        #if defined(DEBUG_PICKING)
            Log.info("n candidates: %d", n);
        #endif
        if (n != -1) {
            int c = candidates[random(n+1)];
            #if defined(DEBUG_PICKING)
                Log.info("final candidate: %d", c);
            #endif
            return c;
        } else {
            return -1;
        }
    } // int pick_any(sandbox, target_color)


    void start_washing(Dot* plan[], Dot* sandbox[]) {
      Log.info("start washing @ (%d,%d)", x, y);
      state = WASHING;
    } // start_washing(plan, sandbox)


    // WASHING: run to the pool, then DUNK
    void wash(Dot* plan[], Dot* sandbox[]) {
      #if defined(TESTING)
          Log.trace("washing @ (%d,%d)", x, y);
      #endif
      if (adjacent(sandbox[POOL_TARGET]) 
          || equals(sandbox[POOL_TARGET])) {
          #if defined(TESTING)
              Log.trace("wash: starting dunk");
          #endif 
          start_dunking(plan, sandbox);
      } else {
          if (!move_djikstra(sandbox[POOL_TARGET], sandbox)) {
              start_resting(plan, sandbox);
          }
      }
    } // wash(plan, sandbox)


    /*
     * DUNKING: I am at the pool
     *   1) swish for a bit
     *   2) release the lock
     *   3) move on to "placing" a thing
     */
    void start_dunking(Dot* plan[], Dot* sandbox[]) {
        if (!acquire(POOL_LOCK, id)) {
            Log.info("Dunking: %d waiting for lock", id);
            wander(sandbox);
            return;
        }
        #if defined(TESTING)
            Log.info("start dunking @ (%d,%d)", x, y);
        #endif
        state = DUNKING;
        dunk_timer->reset();
        swish = 0;
    } // start_dunking(plan, sandbox)


    // DUNKING: just wait a bit
    void  dunk(Dot* plan[], Dot* sandbox[]) {
        #if defined(TESTING)
            Log.trace("dunking");
        #endif
        if (!dunk_timer->isExpired()) {
          int pool_x = sandbox[POOL_TARGET]->x;
          // swish around a lil
          if (swish == 0 || (millis() - swish > 150)) {
            x = (x == pool_x ? pool_x - 1 : pool_x);
            swish = millis();
          }
        } else {
          release(POOL_LOCK);
          start_placing(plan, sandbox);
        }
    } // dunk(plan, sandbox)


    /*
     * PLACING: 
     *   start: I have a thing, probably.  
     *          If nothing needs to get placed, just rest
     */
    void start_placing(Dot* plan[], Dot* sandbox[]) {
        #if defined(TESTING)
            Log.info("start placing @ (%d,%d)", x, y);
        #endif
        target_i = pick_closeish_open(plan, sandbox);
        if (target_i != -1) {
            #if defined(TESTING)
            Log.info("Selected [%d] for placement (%d,%d)", target_i, 
                     plan[target_i]->x, plan[target_i]->y);
            #endif
        }
        state = PLACING;
    } // place(plan, sandbox)


    // PLACING: put a clean food in the target
    void place(Dot* plan[], Dot* sandbox[]) {
      // no target?  rest
      if (target_i == -1 || ! reachable(plan[target_i], sandbox)) {
        wander(sandbox);
        start_resting(plan, sandbox);
        return;
      }
      #if defined(TESTING)
          Log.trace("placing; target_i = %d (%d,%d)", 
                    target_i, plan[target_i]->x, plan[target_i]->y);
      #endif

      Dot* target = plan[target_i];
      if (adjacent(target) || equals(target)) {
          place_brick(target, CLEAN_COLOR, sandbox);
          #if defined(TESTING)
              Log.trace("brick: placed, (%d,%d)", target->x, target->y);
          #endif
          start_resting(plan, sandbox);
      } else {
          if (!move_djikstra(target, sandbox)) {
              start_resting(plan, sandbox);
          }
      }
    } // place(plan, sandbox)


    void start_cleaning(Dot* plan[], Dot* sandbox[]) {
        int i = pick_any(sandbox, DIRTY_COLOR);
        if (i != -1) {
            #if defined(TESTING)
                Log.info("start cleaning: %d @ (%d,%d)", i, 
                          sandbox[i]->x, sandbox[i]->y);
            #endif
            target_i = i;
            state = CLEANING;
            return;
        }
    } // start_cleaning(plan, sandbox)


    void clean(Dot* plan[], Dot* sandbox[]) {
        // no target?  rest
        if (target_i == -1 
            || ! reachable(sandbox[target_i], sandbox)
            || ! sandbox[target_i]->active) {
            #if defined(TESTING)
                Log.info("want to clean, but can't");
                if (target_i != -1) {
                    print_distances(_distances, 
                                    sandbox[target_i]->x, 
                                    sandbox[target_i]->y);
                } else {
                    print_distances(_distances);
                }
            #endif
            wander(sandbox);
            start_resting(plan, sandbox);
            return;
        }

        #if defined(TESTING)
            Log.trace("cleaning; target_i = %d (%d,%d)", 
                      target_i, sandbox[target_i]->x, sandbox[target_i]->y);
        #endif

        Dot* target = sandbox[target_i];
        #if defined(TESTING)
            Log.trace("brick[%d]: (%d,%d)", target_i, target->x, target->y);
        #endif
        if (adjacent(target)) {
            #if defined(TESTING)
                Log.trace("brick (%d,%d): horkt", target->x, target->y);
            #endif
            pick_up(target, sandbox);
            start_washing(plan, sandbox);
        } else {
            if (!move_djikstra(target, sandbox)) {
              start_resting(plan, sandbox);
            }
        }
    } // clean(plan, sandbox)


    void start_wandering(Dot* plan[], Dot* sandbox[]) {
        state = RESTING;
    } // start_wandering(plan, sandbox)

    void start_resting(Dot* plan[], Dot* sandbox[]) {
        state = RESTING;
    } // start_resting(plan, sandbox)


    void rest(Dot* plan[], Dot* sandbox[]) {
        #if defined(TESTING)
            Log.trace("resting");
        #endif
        // are there any DIRTY dots
        int i = pick_any(sandbox, DIRTY_COLOR);
        if (i != -1) {
            #if defined(TESTING)
                Log.info("found some dirty; cleaning");
            #endif
            start_cleaning(plan, sandbox);
            return;
        }

        // are there any MISSING dots
        i = pick_closeish_open(plan, sandbox);
        if (i != -1) {
            #if defined(TESTING)
                Log.info("found %d missing; refilling trash", i);
            #endif
            refill_trash(sandbox);
            // it will get cleaned on the next loop
        }

        if (rest_timer->isExpired()
            && P(25)) {
            wander(sandbox);
        } 
        // "tick"
        if ((millis()/1000 + id%2) % 2) {
            color = color/2;
        }
    } // rest(plan, sandbox)


    // refill the trash "pile" as needed
    void refill_trash(Dot* sandbox[]) {
      #if defined(TESTING)
          Log.info("refill trash");
      #endif
      if (!in(TRASH_X, TRASH_Y, sandbox)) {
          #if defined(TESTING)
              Log.info("trash (%d,%d) not in sandbox", TRASH_X, TRASH_Y);
          #endif
          Dot* trash = activate(sandbox);
          trash->set_color(DIRTY_COLOR);
          trash->x = TRASH_X;
          trash->y = TRASH_Y;
          #if defined(TESTING)
              Log.info("Trash(%d,%d): refilled", trash->x, trash->y);
          #endif
      }
    } // refill_trash(sandbox)


    void post(String message) {
      static unsigned long last_post = 0;
      if (millis() - last_post > 30000) {
        Particle.publish("raccoon", message);
        last_post = millis();
      }
    } // post(message)


    void report(int &n_runs, int &n_rests) {
      static uint32_t last_reported = 0;
      uint32_t elapsed = millis() - last_reported;
      if (elapsed > 300*1000) {
        last_reported = millis();
        double duty_cycle = 100.0*n_runs/(n_rests+n_runs);
        Particle.publish("raccoon", String::format(
              "Running: %d (%5.2f/s), Resting: %d (%5.2f/s), Duty %5.2f%%", 
              n_runs, 1000.0*n_runs/elapsed, 
              n_rests,1000.0*n_rests/elapsed, 
              duty_cycle));
        n_runs = 0;
        n_rests = 0;
      }
    } // report(n_runs, n_rests)


  public:

    Raccoon() : Turtle() {
      state = RESTING;
      step_timer->setInterval(WALK_SPEED/2);
      rest_timer = new SimpleTimer(REST_SPEED);
      dunk_timer = new SimpleTimer(DUNK_SPEED);
      Log.info("creating racc at %lu", millis());
      target = nullptr;
      target_i = -1;
      iq = MATRIX_X + MATRIX_Y;
      budget = 500; // ms
    } // Raccoon()


    void report_state() {
      static int prev_state = 9;
      if (state != prev_state) {
        String s;
        switch (state) {
          case RESTING: 
            s = "resting";
            break;
          case DUNKING:
            s = "dunking";
            break;
          case PLACING:
            s = "placing";
            break;
          case WASHING:
            s = "washing";
            break;
          case CLEANING:
            s = "cleaning";
            break;
          default:
              s = "unknown";
        }
        #if defined(TESTING)
            Log.trace("new state: %s (%d)", s.c_str(), state);
        #endif
        prev_state = state;
      }
    } // report_state()


    void run(Dot* plan[], Dot* sandbox[]) override {
      static int n_runs = 0, n_rests = 0;
      #if defined(TESTING)
          Log.trace(">>>>>>>>>> R@(%d,%d) <<<<<<<<<<", x, y);
      #endif
      if (! step_timer->isExpired()) {
	      return;
      }

      report_state();

      // TODO: figure out why he keeps turning GREEN
      color = RACCOON_COLOR;

      switch (state) {
        case WASHING:
          n_runs ++;
          wash(plan, sandbox);
          break;
        case DUNKING:
          n_runs ++;
          dunk(plan, sandbox);
          break;
        case PLACING:
          n_runs ++;
          place(plan, sandbox);
          break;
        case CLEANING:
          n_runs ++;
          clean(plan, sandbox);
          break;
        case RESTING:
        default:
          n_rests ++;
          rest(plan, sandbox);
      }

      // report(n_runs, n_rests);
    } // run(plan, sandbox)
}; // class Raccoon


int trash_x = 0;

  void update_raccoon_layout(Layout* layout, Dot* sandbox[]) {
    if (layout->show_temperature) {
      // relocate the trashcan
      TRASH_X = 1;
    } else {
      TRASH_X = 0;
    }
    if (layout->show_weather) {
      // move the pool
      sandbox[NRACCOONS]->x = MATRIX_X-3;
      sandbox[NRACCOONS+1]->x = MATRIX_X-2;
    } else {
      sandbox[NRACCOONS]->x = MATRIX_X-2;
      sandbox[NRACCOONS+1]->x = MATRIX_X-1;
    }
  } // update_raccoon_layout(sandbox)


  // update any not-in-plan dots as "dirty"
  // note that the POOL is persistent (NRACCONS+2)
  // trash is NOT PERSISTENT and may get recycled
  void dirty_all_the_things(Dot* plan[], Dot* sandbox[]) {
    for (int i = NRACCOONS+2; i < MAX_DOTS; i++) {
      // if a thing is active and not in the plan, mark it dirty
      if (sandbox[i]->active 
          && !in(sandbox[i], plan)) {
        sandbox[i]->set_color(DIRTY_COLOR);
      }
    }
  } // dirty_all_the_things(sandbox, plan)


  void make_raccoons(Dot* sandbox[]) {
    for (int i = 0; i < NRACCOONS; i++) {
      sandbox[i] = new Raccoon();
    }
    // pool, for washing
    sandbox[NRACCOONS] = new Dot(MATRIX_X-2, MATRIX_Y-1, POOL_COLOR);
    sandbox[NRACCOONS]->active = true;
    sandbox[NRACCOONS+1] = new Dot(MATRIX_X-1, MATRIX_Y-1, POOL_COLOR);
    sandbox[NRACCOONS+1]->active = true;
    sandbox[NRACCOONS+2] = new Dot(TRASH_X, TRASH_Y, DIRTY_COLOR);
    sandbox[NRACCOONS+2]->active = true;
    sandbox[NRACCOONS+3] = new Dot(TRASH_X+1, TRASH_Y, DIRTY_COLOR);
    sandbox[NRACCOONS+3]->active = true;
    for (int i = NRACCOONS + 4; i < MAX_DOTS; i++) {
      sandbox[i] = new Dot();
    }
  } // make_raccoons()


  void loop_raccoons(Dot* plan[], Dot* sandbox[]) {
    // update_raccoon_layout(sandbox);

    // in raccoon mode, food-not-in-plan is "dirty"
    dirty_all_the_things(plan, sandbox);

    for (int i = 0; i < NRACCOONS; i++) {
      #if defined(TESTING)
          Log.trace("loop_racoons: %d", i);
      #endif
      Raccoon* raccoon = (Raccoon*)sandbox[i];
      raccoon->run(plan, sandbox);
    }
    #if defined(TESTING)
        Log.trace("loop_racoons: out");
    #endif
  } // loop_raccoons()
