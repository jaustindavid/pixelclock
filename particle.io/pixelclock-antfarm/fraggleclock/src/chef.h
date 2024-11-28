#ifndef CHEF_H
#define CHEF_H

/*
 * a Chef makes food in the shape of time.
 */

/*
 xxx  x  xxx  xxx  x x  xxx  xxx  xxx  xxx  xxx
 x x  x    x    x  x x  xx   x      x  x x  xxx
 x x  x  xx    xx  xxx    x  xxx   x   xxx    x
 xxx  x  xxx  xxx    x  xxx  xxx   x   xxx  xxx
 */

#include "dot.h"
#include "list.h"
#include "WobblyTime.h"

#undef PRINTF_DEBUGGER

/*
 * Chef renders "food" into time-shaped objects
 *
 * it might be funny about that
 *
 */

#define MODE_NORMAL     0
#define MODE_AMPM       1
#define MODE_OVERFLOW   2
#define MODE_METRIC     3
#define MODE_FRACTIONAL 4
#define MODE_MAX        5  // for incrementing


class Chef {
    private:
        int last_hh, last_mm;
        String chef_time;
        int chaos_mode;
        WobblyTimer *chaotic_timer;
        int font[12][5] = {
            // 0
            {0b0110, 
             0b1001,
             0b1001,
             0b1001,
             0b0110}, 
            // 1
            {0b0010,
             0b0010,
             0b0010,
             0b0010,
             0b0010},
            // 2
            {0b1110,
             0b0001,
             0b0110,
             0b1000,
             0b1111},
             // 3
            {0b1110,
             0b0001,
             0b0110,
             0b0001,
             0b1110},
             // 4
            {0b1001,
             0b1001,
             0b1111,
             0b0001,
             0b0001},
             // 5
            {0b1111,
             0b1000,
             0b1110,
             0b0001,
             0b1110},
             // 6
            {0b0110,
             0b1000,
             0b1110,
             0b1001,
             0b0110},
             // 7
            {0b1111,
             0b0001,
             0b0010,
             0b0100,
             0b0100},
             // 8
            {0b0110,
             0b1001,
             0b0110,
             0b1001,
             0b0110},
             // 9
            {0b0110,
             0b1001,
             0b0111,
             0b0001,
             0b0110},
             // .
            {0b0000,
             0b0000,
             0b0110,
             0b0110,
             0b0000},
             // /
            {0b0000,
             0b0001,
             0b0010,
             0b0100,
             0b1000}
            };


        // maybe returns an updated value for chaos_mode
        // 50%: normal
        // 25%: overflow
        // 15%: ampm
        // 8%:  metric
        // 7%:  fractional
        int be_chaotic(WobblyTime& wTime) {
            if (chaotic_timer->isExpired()) {
                int p = random(100);
                if (p > 50) {
                    return MODE_NORMAL;
                } else if (p > 25) {
                    return MODE_OVERFLOW;
                } else if (p > 15) {
                    return MODE_AMPM;
                } else if (p > 8) {
                    return MODE_METRIC;
                } else {
                    return MODE_FRACTIONAL;
                }
            }
            return chaos_mode;
        } // int be_chaotic(wTime)


        // increments chaos_mode
        int beChaos(String param) {
           chaos_mode = (chaos_mode+1) % MODE_MAX;
            last_mm = -1;
            return chaos_mode;
        }


    public:
        Chef() : last_hh(-1), last_mm(-1), chaos_mode(MODE_NORMAL) {
          chef_time = "00:00";
          // 1-7 days
          chaotic_timer = new WobblyTimer(24*60*60*1000, 7*24*60*60*1000); 
        };
        

        void setup() {
            Particle.function("chef_chaos", &Chef::beChaos, this); 
            Particle.variable("chef_time", this->chef_time);
        }
        

        void prepare(Dot* food[], int d, int dx, int dy) {
            #ifdef PRINTF_DEBUGGER
                Serial.printf("Chef: starting render(%d)\n", d);
            #endif
            for (int y = 0; y < 5; y++) {
                #ifdef PRINTF_DEBUGGER
                    Serial.printf("Scanning %01x\n", font[d][y]);
                #endif
                for (int x = 0; x < 4; x++) {
                    if (font[d][y] & (1 << x)) {  // Check if pixel is set
                        // Draw the pixel
                        #ifdef PRINTF_DEBUGGER
                            Serial.printf("found pixel at (%d,%d)\n", 3-x, y);
                        #endif
                        // food->add(new Dot(3-x+dx, y+dy, GREEN));
                        Dot* dot = activate(food);
                        dot->x = 3-x+dx;
                        dot->y = y+dy;
                        dot->color = GREEN;
                    }
                }
            }
        } // prepare(food, h, dx, dy)
        

        // H H
        // M M
        void cook_normal(Dot* food[], int hh, int mm) {
            prepare(food, hh / 10, 3, 1);
            prepare(food, hh % 10, 9, 1);
            prepare(food, mm / 10, 3, 8);
            prepare(food, mm % 10, 9, 8);
            chef_time = String::format(
                "wobbly %02d:%02d, actual %02d:%02d",
                hh, mm, Time.hour(), Time.minute());
        } // cook_normal(food, wTime)


        // H H
        // M M
        void cook_ampm(Dot* food[], WobblyTime& wTime) {
            int hh = wTime.hour();
            int mm = wTime.minute();
            hh = (hh + 12) % 24;
            cook_normal(food, hh, mm);
            chef_time = String::format(
                "ampm %02d:%02d, actual %02d:%02d",
                hh, mm, Time.hour(), Time.minute());
        } // cook_ampm(food, wTime)


        // H H
        // M M
        void cook_overflow(Dot* food[], WobblyTime& wTime) {
            int hh = wTime.hour();
            int mm = wTime.minute();
            if (mm < 10) {
              mm += 60;
              hh = hh ? hh - 1 : 23;
            }
            cook_normal(food, hh, mm);
            chef_time = String::format(
                "overflow %02d:%02d, actual %02d:%02d",
                hh, mm, Time.hour(), Time.minute());
        } // cook_ampm(food, wTime)
        

        // makes a decimal to the left of normal foods
        void decimate(Dot* food[], int dx, int dy) {
            Dot* dot = activate(food);
            dot->x = dx;  dot->y = dy;
            dot->color = GREEN;
            dot = activate(food);
            dot->x = dx;  dot->y = dy+1;
            dot->color = GREEN;
            dot = activate(food);
            dot->x = dx+1;  dot->y = dy;
            dot->color = GREEN;
            dot = activate(food);
            dot->x = dx+1;  dot->y = dy+1;
            dot->color = GREEN;
        }


        //  H H
        //  . M
        void cook_metric(Dot* food[], WobblyTime& wTime) {
            int hh = wTime.hour();
            int mm = wTime.minute();
            int m = map(mm, 0, 59, 0, 9);
            prepare(food, hh / 10, 3, 1);
            prepare(food, hh % 10, 9, 1);
            decimate(food, 5, 11);
            prepare(food, m, 9, 8);
            chef_time = String::format(
                            "metric %02d.%1d, actual %02d:%02d",
                            hh, m, Time.hour(), Time.minute());
        } // cook_metric(food, wTime)
       

        // makes a slash in the middle of M M 
        void prep_slash(Dot* food[]) {
            Dot* dot = activate(food);
            dot->x = 8;  dot->y = 9; 
            dot->color = GREEN;
            dot = activate(food);
            dot->x = 8;  dot->y = 10; 
            dot->color = GREEN;
            dot = activate(food);
            dot->x = 7;  dot->y = 11; 
            dot->color = GREEN;
            dot = activate(food);
            dot->x = 7;  dot->y = 12; 
            dot->color = GREEN;
        }


        // H H
        // N/D
        void cook_fractional(Dot* food[], WobblyTime& wTime) {
            int hh = wTime.hour();
            int mm = wTime.minute();
            int ss = wTime.second();
            // right-shift by 1/32
            ss += 3600/32;
            int eighths = (60*mm + ss)*8/3600;
            int num = 0;
            int denom = 8;
            if (eighths % 2 == 1) {
              num = eighths;
            } else if (eighths == 2 || eighths == 6) {
              num = eighths/2;
              denom = 4;
            } else if (eighths == 4) {
              num = 1;
              denom = 2;
            }
            prepare(food, hh / 10, 3, 1);
            prepare(food, hh % 10, 9, 1);
            prepare(food, num, 2, 8);
            prep_slash(food);
            prepare(food, denom, 10, 8);
            chef_time = String::format(
                            "fractional %02d:%1d/%1d, actual %02d:%02d",
                            hh, num, denom, Time.hour(), Time.minute());
        } // cook_fractional(food, wTime)


        void cook(Dot* food[], WobblyTime& wTime) {
            int hh = wTime.hour();
            int mm = wTime.minute();
            if ((hh != last_hh) || (mm != last_mm)) {
                #ifdef PRINTF_DEBUGGER
                    Serial.printf("Chef: starting cook(%02d, %02d)\n", hh, mm);
                #endif
                last_hh = hh;
                last_mm = mm;
                // food->clear();
                clear_dots(food);
                if (mm == 0 
                    && chaotic_timer->isExpired()) {
                   chaos_mode = be_chaotic(wTime);
                }

                switch (chaos_mode) {
                    case MODE_AMPM:
                        cook_ampm(food, wTime);
                        break;
                    case MODE_OVERFLOW:
                        cook_overflow(food, wTime);
                        break;
                    case MODE_METRIC:
                        cook_metric(food, wTime);
                        break;
                    case MODE_FRACTIONAL:
                        cook_fractional(food, wTime);
                        break;
                    default:
                        cook_normal(food, hh, mm);
                }
                #ifdef PRINTF_DEBUGGER
                    print_list(food);
                #endif
            }
        }
};

#endif
