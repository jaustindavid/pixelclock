#pragma once

#include "WobblyTime.h"
#include "dot.h"
#include "list.h"
#include "ant.h"

/*
 * A simple display like IN THE MATRIX (spooky sounds)
 */

#define CASCADES_PER_SEC 3
#define DOTS_PER_COLUMN 2

class Matrixel: public Ant {
  private:
    WobblyTimer* cascade_timer;

  public:
    Matrixel(byte _x) {
      x = _x;
      y = random(MATRIX_Y);
      color = TIME_COLOR/2;
      active = true;
      int duration = 1000 / (CASCADES_PER_SEC * MATRIX_Y);
      cascade_timer = new WobblyTimer(duration*3/4, duration*4/3);
    }


    ~Matrixel() {
      delete cascade_timer;
    }


    // cascade
    void run(Dot* plan[], Dot* sandbox[]) override {
      if (! cascade_timer->isExpired()) {
        return;
      }

      // slow down if showing time...
      if (in(this, plan)
          && P(66)) {
        return;
      }

      // drop one, or else reset
      if (y >= MATRIX_Y) {
        x = random(MATRIX_X);
        y = 0;
      } else {
        y += 1;
      }

      // adjust color
      if (in(this, plan)) {
        color = TIME_COLOR;
      } else {
        if (P(25)) {
          color = TIME_COLOR / 2;
        } else {
          lighten(8);
        }
      }
    }
}; // Matrixel


void setup_matrix(Dot* sandbox[]) {
  for (int i = 0; i < MAX_DOTS; i++) {
    sandbox[i] = new Matrixel(random(MATRIX_X));
  }
  return;

  int n = 0;
  for (int i = 0; i < MATRIX_X; i++) {
    for (int j = 0; j < DOTS_PER_COLUMN; j++) {
      sandbox[n] = new Matrixel(i);
      n++;
    }
  }
  for (; n < MAX_DOTS; n++) {
    sandbox[n] = new Dot();
  }
} // setup_matrix(sandbox)


void loop_matrix(Dot* plan[], Dot* sandbox[]) {
  for (int i = 0; i < MAX_DOTS; i++) {
    if (sandbox[i]->active) {
      Matrixel* m = (Matrixel*)sandbox[i];
      m->run(plan, sandbox);
    }
  }
} // loop_matrix(plan, sandbox)
