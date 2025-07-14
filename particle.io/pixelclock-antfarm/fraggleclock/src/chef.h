#pragma once

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
#define MODE_MAX        5  // for cycling
#define MODE_BISCUIT    6
#define MODE_MEETING    7

#define FOOD_COLOR DARKGREEN

// top-left of HH and MM sections
#if (ASPECT_RATIO == SQUARE) 
  #define HH_X 3
  #define HH_Y 1
  #define MM_X HH_X
  #define MM_Y 8
#else
  #ifdef MEGA
    #define HH_X 4
    #define HH_Y 0
  #else
    #define HH_X 5
    #define HH_Y 1
  #endif
  #define MM_X 17
  #define MM_Y HH_Y
#endif

#ifdef MEGA
  #include "font-6x5.h"
#else
  #include "font-5x4.h"
#endif

class Chef {
    private:
        int last_hh, last_mm;
        String chef_time;
        int denominator;
        int chaos_mode;
        typedef uint64_t tracker;
        tracker mode_tracker;
        WobblyTimer *chaotic_timer;

        // returns a value representing a reset
        tracker reset_tracker() {
            return millis();
        } // tracker reset_tracker()


        // true if s secs have elapsed since reset of t
        bool elapsed(tracker t, int s) {
            return s < (int)((millis() - t)/1000);
        } // bool elapsed(s, t)


        // returns to normal, no matter what
        void return_to_normal() {
            chaos_mode = MODE_NORMAL;
            chaotic_timer->reset();
        } // return_to_normal()


        // returns to normal (mode) after s seconds elapsed (in mode_tracker)
        void return_to_normal_after(int s) {
            if (elapsed(mode_tracker, s)) {
                Particle.publish("chef",
                    String::format("returned to normal after %d s", s));
                return_to_normal();
            }
        } // return_to_normal_after(t, s)


        // returns an updated value for chaos_mode
        // 2%:  fractional
        // 3%:  metric
        // 5%:  overflow
        // 10%: ampm
        // 80%: normal
        int be_chaotic() {
            mode_tracker = reset_tracker();
            if (P(2)) {
                denominator = 4+random(6); // 4-9
                Particle.publish("chef", 
                    String::format("new denominator: %d", denominator));
                return MODE_FRACTIONAL;
            } else if (P(3)) {
                return MODE_METRIC;
            } else if (P(5)) {
                return MODE_OVERFLOW;
            } else if (P(10)) {
                return MODE_AMPM;
            } else {
                return_to_normal();
                return MODE_NORMAL;
            }
        } // int be_chaotic(wTime)


        // increments chaos_mode
        int beChaos(String param) {
            if (param.startsWith("biscuit")) {
                chaos_mode = MODE_BISCUIT;
            } else if (param.startsWith("meeting")) {
                chaos_mode = MODE_MEETING;
            } else {
                chaos_mode = (chaos_mode+1) % MODE_MAX;
                last_mm = -1; // force an update
                mode_tracker = reset_tracker();
            }
            return chaos_mode;
        } // int beChaos(param)
        

        void prepare(Dot* food[], int d, int dx, int dy) {
            #ifdef PRINTF_DEBUGGER
                Serial.printf("Chef: starting render(%d)\n", d);
            #endif
            for (int y = 0; y < FONT_HEIGHT; y++) {
                #ifdef PRINTF_DEBUGGER
                    Serial.printf("Scanning %01x\n", font[d][y]);
                #endif
                for (int x = 0; x < FONT_WIDTH; x++) {
                    if (font[d][y] & (1 << x)) {  // Check if pixel is set
                        // Draw the pixel
                        #ifdef PRINTF_DEBUGGER
                            Serial.printf("found pixel at (%d,%d)\n", FONT_WIDTH-1-x, y);
                        #endif
                        Dot* dot = activate(food);
                        dot->x = FONT_WIDTH-1-x+dx;
                        dot->y = y+dy;
                        dot->color = FOOD_COLOR;
                    }
                }
            }
        } // prepare(food, h, dx, dy)


        // H H
        // M M
        void cook_normal(Dot* food[], int hh, int mm) {
            prepare(food, hh / 10, HH_X, HH_Y);// 3, 1);
            prepare(food, hh % 10, HH_X+FONT_WIDTH+1, HH_Y); // 9, 1);
            prepare(food, mm / 10, MM_X, MM_Y); // 3, 8);
            prepare(food, mm % 10, MM_X+FONT_WIDTH+1, MM_Y); // 9, 8);
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


        // vibe-coded with Gemini
        unsigned long simplePRNG(unsigned long seed) {
            // LCG parameters:
            // Multiplier (a)
            const unsigned long a = 16807;
            // Modulus (m = 2^31 - 1, a Mersenne prime)
            const unsigned long m = 2147483647; // 2^31 - 1
            // Increment (c) - set to 0 for a multiplicative LCG
            const unsigned long c = 0;

            // Ensure the seed is within a valid positive range for the modulus.
            // This handles potential negative inputs if the seed calculation
            // ever results in one, and keeps it within the bounds of the
            // modulus.
            seed = (seed % m + m) % m;

            // Calculate the next pseudo-random number
            return (a * seed + c) % m;
        } // simplePRNG(seed)


        bool todayIsAnUpDay() {
          int d = Time.day();
          int m = Time.month();
          int y = Time.year();
          return simplePRNG(d + m + y) % 2 == 0;
        } // todayIsAnUpDay()


        void cook_biscuit(Dot* food[], WobblyTime& wTime) {
            int hh = wTime.hour();
            int mm = wTime.minute();
            if (todayIsAnUpDay()) {
              hh = 23 - hh;
              mm = 59 - mm;
            }
            cook_normal(food, hh, mm);
            chef_time = String::format(
                "biscuit %02d:%02d, actual %02d:%02d",
                hh, mm, Time.hour(), Time.minute());
        } // cook_biscuit(food, wTime)


        void cook_meeting(Dot* food[], WobblyTime& wTime) {
            int hh = wTime.hour();
            int mm = wTime.minute();
            String meeting;
            if (mm == 0) {
                cook_normal(food, 0, 0);
                meeting = "00 00";
            } else if ((mm >= 1 && mm <= 4)) {
                prepare(food, DIGIT_PLUS, HH_X-1, HH_Y);
                prepare(food, 0, HH_X+FONT_WIDTH+1, HH_Y);
                prepare(food, mm / 10, MM_X, MM_Y);
                prepare(food, mm % 10, MM_X+FONT_WIDTH+1, MM_Y);
                meeting = String::format("+0 %02d", mm);
            } else if (mm >= 31 && mm <= 34) {
                int dm = mm - 30;
                prepare(food, DIGIT_PLUS, HH_X-1, HH_Y);
                prepare(food, 0, HH_X+FONT_WIDTH+1, HH_Y);
                prepare(food, dm / 10, MM_X, MM_Y);
                prepare(food, dm % 10, MM_X+FONT_WIDTH+1, MM_Y);
                meeting = String::format("+0 %02d", dm);
            } else if (mm >= 26 && mm <= 29) {
                int dm = 30 - mm;
                prepare(food, DIGIT_MINUS, HH_X-1, HH_Y);
                prepare(food, 0, HH_X+FONT_WIDTH+1, HH_Y);
                prepare(food, dm / 10, MM_X, MM_Y);
                prepare(food, dm % 10, MM_X+FONT_WIDTH+1, MM_Y);
                meeting = String::format("-0 %02d", dm);
            } else if (mm >= 56 && mm <= 59) {
                int dm = 60 - mm;
                prepare(food, DIGIT_MINUS, HH_X-1, HH_Y);
                prepare(food, 0, HH_X+FONT_WIDTH+1, HH_Y);
                prepare(food, dm / 10, MM_X, MM_Y);
                prepare(food, dm % 10, MM_X+FONT_WIDTH+1, MM_Y);
                meeting = String::format("-0 %02d", dm);
            } else {
                cook_normal(food, hh, mm);
            }
            chef_time = String::format(
                "meeting %s, actual %02d:%02d",
                meeting.c_str(), Time.hour(), Time.minute());
        } // cook_meeting(food, wTime)


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
        

        //  H H
        //  . M
        void cook_metric(Dot* food[], WobblyTime& wTime) {
            int hh = wTime.hour();
            int mm = wTime.minute();
            int hundredths = map(mm, 0, 59, 0, 99);
            int m = round(0.1*hundredths);

            if (m == 10) {
              hh = (hh + 1) % 24;
              m = 0;
            }

            prepare(food, hh / 10, HH_X, HH_Y);// 3, 1);
            prepare(food, hh % 10, HH_X+FONT_WIDTH+1, HH_Y); // 9, 1);
            prepare(food, DIGIT_DOT, MM_X, MM_Y+1);
            prepare(food, m, MM_X+FONT_WIDTH+1, MM_Y); // 9, 8);
                                                  //
            chef_time = String::format(
                            "metric %02d.%1d, actual %02d:%02d",
                            hh, m, Time.hour(), Time.minute());
        } // cook_metric(food, wTime)
       

        int gcd(int a, int b) {
            if (b == 0) {
                return a;
            } else {
                return gcd(b, a % b);
            }
        } // int gcd(a, b)


        // H H
        // N/D
        // note that N is never 0 and might be D
        // 7:59, if D = 9, is 7 9/9 and not 8 0/9
        void cook_fractional(Dot* food[], WobblyTime& wTime) {
            int hh = wTime.hour();
            int mm = wTime.minute();
            int ss = wTime.second();
            // right shift by 1/2: ss += 3600/(2*denominator);
            // right-shift by 1
            int numerator = (60*mm + ss)*denominator/3600 + 1;
            // numerator = map(mm, 0, 59, 1, denominator);
            int denom = denominator;

            if (numerator != denominator) {
                // reduce the fraction
                int gcd_value = gcd(numerator, denominator);
                numerator /= gcd_value;
                denom /= gcd_value;
            }

            prepare(food, hh / 10, HH_X-1, HH_Y);// 3, 1);
            prepare(food, hh % 10, HH_X+FONT_WIDTH+1, HH_Y); // 9, 1);
            prepare(food, numerator, MM_X-1, MM_Y); // MM_X, left 1
            prepare(food, DIGIT_SLASH, MM_X+FONT_WIDTH-2, MM_Y); 
            prepare(food, denom, MM_X+FONT_WIDTH+2, MM_Y);  // MM_X, right 2
            chef_time = String::format(
                            "fractional %02d:%1d/%1d, actual %02d:%02d",
                            hh, numerator, denom,
                            Time.hour(), Time.minute());
        } // cook_fractional(food, wTime)


        void test_chaos() {
            int n[MODE_MAX] = { 0, 0, 0, 0, 0 };
            for (int i = 0; i < 1000; i++) {
              n[be_chaotic()] ++;
            }

            String s = String::format("chaos[");

            for (int i = 0; i < MODE_MAX; i++) {
              s.concat(String::format("%5.2f%% ", 0.1*n[i]));
            }

            Particle.publish("chaos test", s);
        } // test_chaos


    /*
     * needed: managed chaos
     *   % odds it will start
     *   conditions for it to end
     */

        void manage_chaos(Dot* food[], WobblyTime& wTime) {
            switch (chaos_mode) {
                case MODE_AMPM:
                    cook_ampm(food, wTime);
                    // only do this for 30 minutes
                    return_to_normal_after(30*60);
                    break;
                case MODE_OVERFLOW:
                    cook_overflow(food, wTime);
                    // stop overflowing at 10th minute
                    if (wTime.minute() == 10) {
                        return_to_normal();
                    }
                    break;
                case MODE_METRIC:
                    cook_metric(food, wTime);
                    // only do this for 12 hours
                    if (wTime.minute() == 0) {
                        return_to_normal_after(12*3600);
                    }
                    break;
                case MODE_FRACTIONAL:
                    cook_fractional(food, wTime);
                    // only do this for 2 hours
                    if (wTime.minute() == 0) {
                        return_to_normal_after(2*3600);
                    }
                    break;
                case MODE_BISCUIT:
                    cook_biscuit(food, wTime);
                    // NEVER STOP
                    break;
                case MODE_MEETING:
                    cook_meeting(food, wTime);
                    // NEVER STOP
                    break;
                default:
                    cook_normal(food, wTime.hour(), wTime.minute());
                    if (chaotic_timer->isExpired()) {
                        // test_chaos(); 
                        chaos_mode = be_chaotic();
                        Particle.publish("chef", 
                            String::format("new chaos mode: %d", chaos_mode));
                    }
            }
        } // manage_chaos(food, wTime)


    public:

        Chef() : last_hh(-1), 
                 last_mm(-1), 
                 denominator(8), 
                 chaos_mode(MODE_NORMAL) {
          chef_time = "00:00";
          // 1-7 days
          chaotic_timer = new WobblyTimer(24*60*60*1000, 7*24*60*60*1000); 
          // chaotic_timer = new WobblyTimer(1*60*1000, 5*60*1000); 
          mode_tracker = reset_tracker();
        };
        

        void setup_cloud() {
            Particle.function("chef_chaos", &Chef::beChaos, this); 
            Particle.variable("chef_time", this->chef_time);
        } // setup_cloud()


        void cook(Dot* food[], WobblyTime& wTime) {
            int hh = wTime.hour();
            int mm = wTime.minute();
            if ((hh != last_hh) || (mm != last_mm)) {
                #ifdef PRINTF_DEBUGGER
                    Serial.printf("Chef: starting cook(%02d, %02d)\n", hh, mm);
                #endif
                // Particle.publish("chef", 
                //         String::format("boop.  mode = %d", chaos_mode));
                last_hh = hh;
                last_mm = mm;
                // food->clear();
                clear_dots(food);

                manage_chaos(food, wTime);

                #ifdef PRINTF_DEBUGGER
                    print_list(food);
                #endif
            }
        } // cook()
}; // class Chef
