#pragma once

#include "WobblyTime.h"
#include "dot.h"
#include "list.h"
#include "ant.h"

/*
 * A simple display like IN THE MATRIX (spooky sounds)
 */

#define CASCADES_PER_SEC 4
#define DOTS_PER_COLUMN 2
#define NMATRIXELS 75

class Matrixel: public Ant {
  private:
    WobblyTimer* cascade_timer;
    byte x_min, x_max, y_min, y_max;

  public:
    // min & max are INCLUSIVE
    Matrixel(byte _x_min, byte _x_max, byte _y_min, byte _y_max) {
      layout(_x_min, _x_max, _y_min, _y_max);
      x = random(x_min, x_max+1);
      y = random(y_min, y_max+1);
      color = TIME_COLOR/2;
      active = true;
      int duration = 1000 / (CASCADES_PER_SEC * (y_max - y_min));
      cascade_timer = new WobblyTimer(duration*3/4, duration*4/3);
    }


    ~Matrixel() {
      delete cascade_timer;
    }


    void layout(byte _x_min, byte _x_max, byte _y_min, byte _y_max) {
      x_min = _x_min;
      x_max = _x_max;
      y_min = _y_min;
      y_max = _y_max;
    } // layout(x, x, y, y)


    // fall until finding a clear spot
    void cascade(Dot* sandbox[]) {
      byte target_x = x;
      byte target_y = y;
      while (in(target_x, target_y, sandbox)) {
        target_y += 1;
        if (target_y > y_max) {
          target_x = random(x_min, x_max+1);
          target_y = y_min;
        }
      }
      x = target_x;
      y = target_y;
    } // cascade(sandbox)


    // if I am in the plan, maybe hang out a lil
    // otherwise cascade
    void run(Dot* plan[], Dot* sandbox[]) override {
      if (! cascade_timer->isExpired()) {
        return;
      }

      // if showing time... maybe wait 
      if (in(this, plan) && P(33)) {
        return;
      }

      // cascade
      cascade(sandbox);

      // adjust color
      if (in(this, plan)) {
        color = TIME_COLOR;
      } else {
        color = NITE_COLOR;
        if (P(25)) {
          lighten(8);
        }
      }
    }
}; // Matrixel


class SandboxManager {
  public:
    virtual void setup(Dot* _sandbox[]);
    virtual void layout(Layout* layout, Dot* sandbox[]);
    virtual void loop(Dot* plan[], Dot* sandbox[]);
};


class MatrixManager: protected SandboxManager {
  private:
    byte min_x, max_x, min_y, max_y;

  public:
    void setup(Dot* sandbox[]) {
      min_x = 0;
      max_x = MATRIX_X - 1;
      min_y = 0;
      max_y = MATRIX_Y - 1;

      for (int i = 0; i < NMATRIXELS; i++) {
        sandbox[i] = new Matrixel(min_x, max_x, min_y, max_y);
      }
    } // setup()


    void layout(Layout* layout, Dot* sandbox[]) {
      if (layout->show_pinger) {
        max_x = MATRIX_X - 2;
      } else {
        max_x = MATRIX_X - 1;
      }

      if (layout->show_temperature) {
        min_y = 1;
      } else {
        min_y = 0;
      }

      for (int i = 0; i < NMATRIXELS; i++) {
        Matrixel* m = (Matrixel*)sandbox[i];
        m->layout(min_x, min_y, max_x, max_y);
      }
    } // layout(layout)


    void loop(Dot* plan[], Dot* sandbox[]) {
      for (int i = 0; i < MAX_DOTS; i++) {
        if (sandbox[i]->active) {
          Matrixel* m = (Matrixel*)sandbox[i];
          m->run(plan, sandbox);
        }
      }
    } // loop(plan)
};


void setup_matrix(Dot* sandbox[]) {
  for (int i = 0; i < MAX_DOTS; i++) {
    sandbox[i] = new Matrixel(1, MATRIX_X-2, 0, MATRIX_Y-1);
  }
} // setup_matrix(sandbox)


void layout_matrix(Layout* layout, Dot* sandbox[]) {
  byte min_x, max_y;

  if (layout->show_pinger) {
    max_y = MATRIX_Y - 2;
  } else {
    max_y = MATRIX_Y - 1;
  }

  if (layout->show_temperature) {
    min_x = 1;
  } else {
    min_x = 0;
  }

  for (int i = 0; i < NMATRIXELS; i++) {
    Matrixel* m = (Matrixel*)sandbox[i];
    m->layout(min_x, MATRIX_X-1, 0, max_y);
  }
} // layout_matrix(layoutm sandbox)


void loop_matrix(Dot* plan[], Dot* sandbox[]) {
  for (int i = 0; i < MAX_DOTS; i++) {
    if (sandbox[i]->active) {
      Matrixel* m = (Matrixel*)sandbox[i];
      m->run(plan, sandbox);
    }
  }
} // loop_matrix(plan, sandbox)
