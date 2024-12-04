#ifndef _RACCOON_H_
#define _RACCOON_H_


#include <SimpleTimer.h>
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
#define WASHING SPAZZING // 3
// cleaning 4
#define DUNKING DUMPING // 5

#define NRACCOONS 1

#define WALK_SPEED  250 // ms per step
#define REST_SPEED 1000 // ms per step
#define DUNK_SPEED 1500 // ms per dunking

#define DIRTY_COLOR (Adafruit_NeoPixel::Color(192, 64, 0))
#define CLEAN_COLOR GREEN

#define POOL_TARGET 2 // sandbox index for a pool node
#define POOL_COLOR (Adafruit_NeoPixel::Color(32, 32, 192))

class Raccoon: public Turtle {
  private:
    Dot* target;
    SimpleTimer *rest_timer;
    SimpleTimer *dunk_timer;
    uint32_t swish;


    // returns a dot of target_color, or -1
    int pick_any(Dot* sandbox[], color_t target_color) {
       for (int i = NRACCOONS+2; i < MAX_DOTS; i++) {
         if (sandbox[i]->active 
             && sandbox[i]->color == target_color) {
           return i;
         }
       }
       return -1;
    } // int pick_any(sandbox, target_color)


    void start_washing(Dot* plan[], Dot* sandbox[]) {
      state = WASHING;
    } // start_washing(plan, sandbox)


    // WASHING: run to the pool, then DUNK
    void wash(Dot* plan[], Dot* sandbox[]) {
      Log.trace("washing");
      if (adjacent(sandbox[POOL_TARGET]) 
          || equals(sandbox[POOL_TARGET])) {
        start_dunking(plan, sandbox);
        return;
      } 

      if (move_toward(sandbox[POOL_TARGET], sandbox)) {
        return;
      }

      wander(sandbox);
    } // wash(plan, sandbox)


    void start_dunking(Dot* plan[], Dot* sandbox[]) {
      state = DUNKING;
      dunk_timer->reset();
      swish = 0;
    } // start_dunking(plan, sandbox)


    // DUNKING: just wait a bit
    void dunk(Dot* plan[], Dot* sandbox[]) {
      Log.trace("dunking");
      if (dunk_timer->isExpired()) {
        start_placing(plan, sandbox);
      } else {
        // swish around a lil
        if (swish == 0 || (millis() - swish > 150)) {
          x = (x == 14 ? 15 : 14);
          swish = millis();
        }
      }
    } // dunk(plan, sandbox)


    void start_placing(Dot* plan[], Dot* sandbox[]) {
        target_i = pick_closeish_open(plan, sandbox);
        if (target_i != -1) {
          Log.trace("Selected [%d] for placement (%d,%d)", target_i, 
                    plan[target_i]->x, plan[target_i]->y);
        }
        state = PLACING;
    } // place(plan, sandbox)


    // PLACING: put a clean food in the target
    void place(Dot* plan[], Dot* sandbox[]) {
      Log.trace("placing; target_i = %d", target_i);
      // no target?  rest
      if (target_i == -1) {
        start_resting(plan, sandbox);
        return;
      }

      Dot* target = plan[target_i];
      if (adjacent(target)) {
        place_brick(target, CLEAN_COLOR, sandbox);
        Log.trace("brick: placed, (%d,%d)", target->x, target->y);
        start_resting(plan, sandbox);
      } else {
        move_toward(target, sandbox);
      }
    } // place(plan, sandbox)


    void start_cleaning(Dot* plan[], Dot* sandbox[]) {
      int i = pick_any(sandbox, DIRTY_COLOR);
      if (i != -1) {
        Log.info("start cleaning: %d @ (%d,%d)", i, 
                  sandbox[i]->x, sandbox[i]->y);
        target_i = i;
        state = CLEANING;
        return;
      }
    } // start_cleaning(plan, sandbox)


    void clean(Dot* plan[], Dot* sandbox[]) {
      Log.trace("cleaning; target_i = %d", target_i);
      // no target?  rest
      if (target_i == -1) {
        start_resting(plan, sandbox);
        return;
      }

      Dot* target = sandbox[target_i];
      if (adjacent(target)) {
        Log.trace("brick (%d,%d): horkt", target->x, target->y);
        pick_up(target, sandbox);
        start_washing(plan, sandbox);
      } else {
        move_toward(target, sandbox);
      }
    } // clean(plan, sandbox)


    void start_resting(Dot* plan[], Dot* sandbox[]) {
      state = RESTING;
    } // rest(plan, sandbox)


    void rest(Dot* plan[], Dot* sandbox[]) {
      Log.trace("resting");
      // are there any DIRTY dots
      int i = pick_any(sandbox, DIRTY_COLOR);
      if (i != -1) {
        Log.info("found some dirty; cleaning\n", i);
        start_cleaning(plan, sandbox);
        return;
      }

      // are there any MISSING dots
      i = pick_closeish_open(plan, sandbox);
      if (i != -1) {
        Log.info("found %d missing; washing\n", i);
        start_washing(plan, sandbox);
      }

      wander(sandbox);
    } // rest(plan, sandbox)


  public:
    Raccoon() : Turtle() {
      color = DARKWHITE;
      state = RESTING;
      step_timer->setInterval(WALK_SPEED/2);
      rest_timer = new SimpleTimer(REST_SPEED);
      dunk_timer = new SimpleTimer(DUNK_SPEED);
      Log.info("creating racc at %lu", millis());
      target = nullptr;
      target_i = -1;
    }

    
    void post(String message) {
      static unsigned long last_post = 0;
      if (millis() - last_post > 30000) {
        Particle.publish("raccoon", message);
        last_post = millis();
      }
    } // post(message)


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
        Log.trace("new state: %s (%d)", s.c_str(), state);
        prev_state = state;
      }
    } // report_state()


    void run(Dot* plan[], Dot* sandbox[]) override {
      if (! step_timer->isExpired()) {
	      return;
      }

      report_state();

      switch (state) {
        case WASHING:
          wash(plan, sandbox);
          break;
        case DUNKING:
          dunk(plan, sandbox);
          break;
        case PLACING:
          place(plan, sandbox);
          break;
        case CLEANING:
          clean(plan, sandbox);
          break;
        case RESTING:
        default:
          rest(plan, sandbox);
      }
    } // run(plan, sandbox)
}; // class Raccoon


  void dirty_all_the_things(Dot* plan[], Dot* sandbox[]) {
    for (int i = NRACCOONS+2; i < MAX_DOTS; i++) {
      // if a thing is active and not in the plan, mark it dirty
      if (sandbox[i]->active 
          && !in(sandbox[i], plan)) {
        sandbox[i]->set_color(DIRTY_COLOR);
      }
    }
  } // dirty_all_the_things(sandbox, plan)

#endif
