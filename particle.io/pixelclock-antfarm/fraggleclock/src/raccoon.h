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
// cleaning 3
#define DUNKING DUMPING // 4
#define WASHING 5

#define NRACCOONS 1
#define RACCOON_COLOR (Adafruit_NeoPixel::Color(96, 96, 96))

#if (ASPECT_RATIO == SQUARE)
  #define WALK_SPEED  400 // ms per step
  #define REST_SPEED 1000 // ms per step
  #define DUNK_SPEED 1500 // ms per dunking
#else
  #define WALK_SPEED  300 // ms per step
  #define REST_SPEED 1000 // ms per step
  #define DUNK_SPEED 1250 // ms per dunking
#endif

#define DIRTY_COLOR (Adafruit_NeoPixel::Color(192, 64, 0))
#define CLEAN_COLOR GREEN

#define POOL_TARGET 2 // sandbox index for a pool node
#define POOL_COLOR (Adafruit_NeoPixel::Color(32, 32, 192))


class Stopwatch {
  private:
    uint32_t counter;
    uint32_t start_ms;

  public:
    Stopwatch() {
      reset();
    } // Stopwatch()

    void reset() {
      start_ms = 0;
      counter = 0;
    } // reset()

    void start() {
      start_ms = millis();
    }  // start()

    void stop() {
      if (start_ms) {
        counter += (millis() - start_ms);
        start_ms = 0;
      }
    } // stop()

    uint32_t read() {
      return counter;
    }
};


class Raccoon: public Turtle {
  private:
    Dot* target;
    SimpleTimer *rest_timer;
    SimpleTimer *dunk_timer;
    uint32_t swish;
    Stopwatch rest_counter, run_counter;
    int TRASH_X, TRASH_Y;


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
      Log.info("washing @ (%d,%d)", x, y);
      if (adjacent(sandbox[POOL_TARGET]) 
          || equals(sandbox[POOL_TARGET])) {
        Log.trace("wash: starting dunk");
        start_dunking(plan, sandbox);
        return;
      } 


      Log.trace("washing: tryna move_toward");
      if (move_toward(sandbox[POOL_TARGET], sandbox)) {
        Log.trace("washing: moved! now @ (%d,%d)", x, y);
        return;
      }

      Log.trace("washing: move failed :( still @ (%d,%d)", x, y);
      wander(sandbox);
      Log.trace("washing: done, now at @ (%d,%d)", x, y);
    } // wash(plan, sandbox)


    void start_dunking(Dot* plan[], Dot* sandbox[]) {
      state = DUNKING;
      dunk_timer->reset();
      swish = 0;
    } // start_dunking(plan, sandbox)


    // DUNKING: just wait a bit
    void dunk(Dot* plan[], Dot* sandbox[]) {
      Log.info("dunking");
      if (dunk_timer->isExpired()) {
        start_placing(plan, sandbox);
      } else {
        // swish around a lil
        if (swish == 0 || (millis() - swish > 150)) {
          x = (x == MATRIX_X-2 ? MATRIX_X-1 : MATRIX_X-2);
          swish = millis();
        }
      }
    } // dunk(plan, sandbox)


    void start_placing(Dot* plan[], Dot* sandbox[]) {
        target_i = pick_closeish_open(plan, sandbox);
        if (target_i != -1) {
          Log.info("Selected [%d] for placement (%d,%d)", target_i, 
                   plan[target_i]->x, plan[target_i]->y);
        }
        state = PLACING;
    } // place(plan, sandbox)


    // PLACING: put a clean food in the target
    void place(Dot* plan[], Dot* sandbox[]) {
      // no target?  rest
      if (target_i == -1) {
        start_resting(plan, sandbox);
        return;
      }
      Log.info("placing; target_i = %d (%d,%d)", 
               target_i, plan[target_i]->x, plan[target_i]->y);

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
      // no target?  rest
      if (target_i == -1) {
        start_resting(plan, sandbox);
        return;
      }

      Log.info("cleaning; target_i = %d (%d,%d)", 
               target_i, sandbox[target_i]->x, sandbox[target_i]->y);

      Dot* target = sandbox[target_i];
      Log.trace("brick[%d]: (%d,%d)", target_i, target->x, target->y);
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
        Log.info("found some dirty; cleaning");
        start_cleaning(plan, sandbox);
        return;
      }


      // are there any MISSING dots
      i = pick_closeish_open(plan, sandbox);
      if (i != -1) {
        // Log.info("found %d missing; washing\n", i);
        // start_washing(plan, sandbox);
        Log.info("found %d missing; refilling trash\n", i);
        refill_trash(sandbox);
        // it will get cleaned on the next loop
      }

      if (rest_timer->isExpired()) {
        wander(sandbox);
      }
    } // rest(plan, sandbox)


    // refill the trash "pile" as needed
    void refill_trash(Dot* sandbox[]) {
      if (!in(TRASH_X, TRASH_Y, sandbox)) {
        Dot* trash = activate(sandbox);
        trash->set_color(DIRTY_COLOR);
        trash->x = TRASH_X;
        trash->y = TRASH_Y;
        Log.trace("Trash(%d,%d): refilled", trash->x, trash->y);
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
      color = RACCOON_COLOR;
      state = RESTING;
      step_timer->setInterval(WALK_SPEED/2);
      rest_timer = new SimpleTimer(REST_SPEED);
      dunk_timer = new SimpleTimer(DUNK_SPEED);
      Log.info("creating racc at %lu", millis());
      target = nullptr;
      target_i = -1;
      TRASH_X = 0;
      TRASH_Y = MATRIX_Y - 1;
      Log.info("Raccoon; trash@(%d,%d)", TRASH_X, TRASH_Y);
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
        Log.trace("new state: %s (%d)", s.c_str(), state);
        prev_state = state;
      }
    } // report_state()


    void run(Dot* plan[], Dot* sandbox[]) override {
      static int n_runs = 0, n_rests = 0;
      Log.trace(">>>>>>>>>> R@(%d,%d) <<<<<<<<<<", x, y);
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

      report(n_runs, n_rests);
    } // run(plan, sandbox)
}; // class Raccoon




  // update any not-in-plan dots as "dirty"
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
    sandbox[0] = new Raccoon();
    // pool, for washing
    sandbox[1] = new Dot(MATRIX_X-2, MATRIX_Y-1, POOL_COLOR);
    sandbox[2] = new Dot(MATRIX_X-1, MATRIX_Y-1, POOL_COLOR);
    sandbox[3] = new Dot(0, MATRIX_Y-1, DIRTY_COLOR);
    sandbox[4] = new Dot(1, MATRIX_Y-1, DIRTY_COLOR);
    sandbox[1]->active = true;
    sandbox[2]->active = true;
    sandbox[3]->active = true;
    sandbox[4]->active = true;
    for (int i = NRACCOONS + 4; i < MAX_DOTS; i++) {
      if (i < NRACCOONS) {
      } else {
        sandbox[i] = new Dot();
      }
    }
  } // make_raccoons()


  void loop_raccoons(Dot* plan[], Dot* sandbox[]) {
    // in raccoon mode, food-not-in-plan is "dirty"
    dirty_all_the_things(plan, sandbox);

    for (int i = 0; i < NRACCOONS; i++) {
      Raccoon* raccoon = (Raccoon*)sandbox[i];
      raccoon->run(plan, sandbox);
    }
  } // loop_raccoons()
#endif
