#pragma once

/*
 * usage:
 *   ::update(icon)
 *   display->render(::dots(), ::ndots)
 */

#include "color.h"
#include "ant.h"


#define CLEAR            1
#define FEW_CLOUDS       2
#define SCATTERED_CLOUDS 3
#define BROKEN_CLOUDS    4
#define SHOWER_RAIN      9
#define RAIN            10
#define THUNDERSTORM    11
#define MIST            50
#define NIGHT          100
#define DAY            101


class WeatherBug: public Ant {
  private:

    void run_clear(byte condition, byte tod, byte hh, byte mm) {
      byte height;
      if (tod == NIGHT) {
        if (hh > 12) {
          height = map(hh, 18, 24, MATRIX_Y-1, 0);
        } else {
          height = map(hh, 0, 8, 0, MATRIX_Y-1);
        }
        if (y == height || y == height + 1) {
          color = MIDWHITE;
        } else {
          // TODO: twinkle?  cloudy?
          color = BLACK;
        }
      } else {
        height = constrain(abs(12-hh), 0, MATRIX_Y-3);
        if (y == height || y == height + 1) {
          color = YELLOW;
        } else {
          if (y >= MATRIX_Y-3) {
            color = GREEN;
          } else {
            color = LIGHTBLUE;
          }
        }
      }
    } // run_clear(condition, tod, hh, mm)


    void run_rain(byte condition, byte tod, byte hh, byte mm) {
      // fall, rate defined by magnitude of condition
      color = P(50) ? 0x000044 : 0x000066;
      bool moving = false;
      switch (condition) {
        case SHOWER_RAIN:
          moving = P(10);
          break;
        case RAIN:
          moving = P(30);
          break;
        case THUNDERSTORM:
        default:
          moving = P(60);
      }
      if (moving) {
        y = (y+1) % MATRIX_Y;
      }
    } // run_rain(condition, tod, hh, mm)


  public:
    WeatherBug() : Ant() {
      active = false;
    } // WeatherBug()


    void run(byte condition, byte tod, byte hh, byte mm) {
      Log.trace("bug(%d,%d)", x, y);
      switch (condition) {
        case SHOWER_RAIN:
        case RAIN:
        case THUNDERSTORM:
          run_rain(condition, tod, hh, mm);
          break;
        case CLEAR:
        case FEW_CLOUDS:
        case SCATTERED_CLOUDS:
        case BROKEN_CLOUDS:
        default:
          run_clear(condition, tod, hh, mm);
      }
    }
}; // WeatherBug


class WeatherGrafix {
  private:
    WeatherBug* pixels[MATRIX_X];
    String icon;
    byte hh, mm;
    byte tod;
    byte condition;


    // returns true if something changed
    bool interpret_icon() {
      bool delta = false;
      if (icon.endsWith("n") 
          && tod != NIGHT) {
        delta = true;
        tod = NIGHT;
      } else if (icon.endsWith("d")
                 && tod != DAY) {
        delta = true;
        tod = DAY;
      }

      if (icon.toInt() != condition) {
        delta = true;
        condition = icon.toInt();
      }

      return delta;
    } // interpret_icon()


    void make_vertical() {
      for (int i = 0; i < MATRIX_X; i++) {
        WeatherBug* bug = (WeatherBug*)pixels[i];
        if (i < MATRIX_Y) {
          bug->x = MATRIX_X-1;
          bug->y = i;
        } else {
          bug->active = false;
        } 
      }
    } // make_vertical()


    void make_horizontal() {
      for (int i = 0; i < MATRIX_X; i++) {
        WeatherBug* bug = (WeatherBug*)pixels[i];
        bug->y = 0;
        bug->x = i;
        bug->active = true;
      }
    } // make_horizontal()


    void reconfigure_pixels() {
      switch (condition) {
        case SHOWER_RAIN:
        case RAIN:
        case THUNDERSTORM:
        case MIST:
          make_horizontal();
          break;
        case CLEAR:
        default:
          make_vertical();
      }
    } // reconfigure_pixels()


  public:
    WeatherGrafix() {
      for (int i = 0; i < MATRIX_X; i++) {
        pixels[i] = new WeatherBug();
        pixels[i]->x = pixels[i]->y = i;
        pixels[i]->active = true;
      }
    } // WeatherGrafix()


    void setup_cloud() {
      #ifdef DEBUG_GFX
      Particle.variable("gfx_icon", this->icon);
      #endif
    } // setup_cloud()


    void update(byte new_hh, byte new_mm, String new_icon) {
      if (icon != new_icon) {
        // switch mode?
        icon = new_icon;
        if (interpret_icon()) {
          Log.warn("reconfiguring weather graphic; icon=%s", icon.c_str());
          reconfigure_pixels();
        }
      }
      hh = new_hh;
      mm = new_mm;

      for (int i = 0; i < MATRIX_X; i++) {
        WeatherBug* bug = (WeatherBug*)pixels[i];
        if (bug->active) {
          bug->run(condition, tod, hh, mm);
        }
      }
    } // update(hh, mm, icon)


    Dot** dots() {
      return (Dot**)pixels;
    }


    int ndots() {
      return MATRIX_X;
    }

}; // WeatherGrafix
