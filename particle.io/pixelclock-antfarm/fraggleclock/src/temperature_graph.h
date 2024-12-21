#ifndef TEMP_GRAPH_H
#define TEMP_GRAPH_H

#include "defs.h"
#include "color.h"
#include "dot.h"

/*
 * a vertical graph showing the temperature
 */
class TemperatureGraph {
  private:
    byte x;


    // what color does this feel like?
    color_t feels_like_color(int temp) {
      if (temp < 10) {
        return BLUE;
      } else if (temp < 20) {
        return LIGHTBLUE;
      } else if (temp < 28) {
        return YELLOW;
      } else if (temp < 35) {
        return ORANGE;
      } else {
        return RED;
      }
    } // color_t feels_like_color(temp)


  public:
    Dot* dots[MATRIX_Y];

    TemperatureGraph(byte new_x) {
      int fake_temp;
      x = new_x;
      for (int i = 0; i < MATRIX_Y; i++) {
        fake_temp = map(i, MATRIX_Y-1, 0, 0, 40);
        dots[i] = new Dot(x, i, feels_like_color(fake_temp));
        dots[i]->active = false;
      }
    } // TemperatureGraph


    // updates the graph to reflect what the temp feels_like (C)
    void update(int feels_like) {
      // counting up from bottom: 0 - 40C
      int mapped_temp = map(feels_like, 0, 40, MATRIX_Y, 0);
      for (int y = 0; y < MATRIX_Y; y++) { 
        if (mapped_temp < y) {
          dots[y]->active = true;
        } else {
          dots[y]->active = false;
        }
      }
    } // void update(feels_like)
}; // class TemperatureGraph

#endif
