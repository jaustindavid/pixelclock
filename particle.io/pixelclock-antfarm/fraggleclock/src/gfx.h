#ifndef _GFX_H_
#define _GFX_H_

#undef DEBUG_GFX

/*
 * this class will control several pixels 
 * and will animate them based on (provided) temp & icon
 *
 * temp: feels_like, degrees C
 * icon: open weather icons, which are %02d%c
 *       https://openweathermap.org/weather-conditions
 *
 * ideas:
 *   sunny: Y Y B B B B G G (sun over grass)
 *   partly cloudy: a few Ws bouncing around over the Y & B 
 *       (or Y & Bs deciding to turn W randomly)
 *   rainy: Bs falling down
 */
 
 #include "color.h"
 #include "ant.h"
 #include "WobblyTime.h"
 
 /*
  * a WeatherBug can act like a raindrop, sunbeam, etc
  */
class WeatherBug: public Ant {
    private:
        int mode; 
        WobblyTime* wTime;
        #define SUNNY_MODE 1
        #define CLOUDY_MODE 4
        #define LIGHTRAIN_MODE 8
        #define HARDRAIN_MODE 10
        #define LIGHTNING_MODE 11
        #define NIGHT_MODE 20
        #define UNKNOWN_MODE 0

    public:
        WeatherBug(WobblyTime* _wTime) : Ant() {
            mode = SUNNY_MODE;
            active = true;
            wTime = _wTime;
            x = MATRIX_X-1;
            y = 0;
        }
        
        
        void setMode(int newMode) {
            mode = newMode;
            switch (mode) {
                case SUNNY_MODE:
                case CLOUDY_MODE:
                case NIGHT_MODE:
                    active = true;
                    break;
                case LIGHTRAIN_MODE:
                    active = P(25);
                    break;
                case HARDRAIN_MODE:
                    // FALLTHROUGH
                case LIGHTNING_MODE:
                    active = P(50);
                    break;
                default:
                    active = true;
                    color = BLACK;
            }   
        }


        // bottom -> top
        // GG B..SS..B
        void be_sunny() {
            // bottom two are GG
            if (y >= MATRIX_Y-2) {
                color = GREEN;
            } else {
                int height = constrain(abs(12-wTime->hour()), 0, MATRIX_Y-3);
                if (y == height || y == height+1) {
                    color = YELLOW;
                } else {
                    color = BLUE;
                }
            }
        } // be_sunny()
        
    
        // moon height: distance from midnight
        void be_nighted() {
            int height;
            if (wTime->hour() > 12) {
                height = map(wTime->hour(), 18, 24, MATRIX_Y-1, 0);
            } else {
                height = map(wTime->hour(), 0, 8, 0, MATRIX_Y-1);
            }
            if (y == height || y == height + 1) {
                color = MIDWHITE;
            } else if (color == BLACK) {
                // maybe make a twinkle
                if (P(1) && (millis()/1000%2==0)) {
                    int w = 32+random(64);
                    color = Adafruit_NeoPixel::Color(w, w, w);
                }
            } else if (P(20) || color == WHITE) {
                color = BLACK;
            } else {
                color += Adafruit_NeoPixel::Color(4, 4, 4);
            }
        }


        // 50% chance of falling "down"
        void be_rain(int p, Dot* peers[]) {
            return;
            color = BLUE;
            return;
            if (P(p)) {
                if (y >= MATRIX_Y) {
                    y = 0;
                } else {
                    step(0, 1, peers);
                }
            }
        }


        // p == 0 no clouds
        // p == 2 all clouds
        void be_cloudy(int p) {
            for (int i = 0; i < 3; i++) {
                if (i < p) {
                    if ((y == i) || (y == i+3)) {
                        lighten(p*32);
                    }
                } 
            }
        }
        
        
        void run(Dot* peers[]) {
            switch (mode) {
                case SUNNY_MODE:
                    be_sunny();
                    break;
                case NIGHT_MODE:
                    be_nighted();
                    break;
                case LIGHTRAIN_MODE:
                    be_rain(25, peers);
                    break;
                case HARDRAIN_MODE:
                case LIGHTNING_MODE:
                    be_rain(75, peers);
                    break;
                case CLOUDY_MODE:
                    be_sunny();
                    be_cloudy(1);
                    break;
                default:
                    color = BLACK;
            }    
        }
};


class WeatherGFX {
    private:
        String icon_s;
        WobblyTime* wTime;
        

        void setMode(int newMode) {
            mode = newMode;
            for (int i = 0; i < MATRIX_Y; i++) {
                WeatherBug* bug = (WeatherBug*)peers[i];
                bug->setMode(newMode);
                bug->y = i;
            }
        }


        void update(String icon) {
            icon_s = icon;
            int i = icon_s.toInt();
            char c = icon_s.charAt(2);
            if ((icon_i != i) || (icon_c != c)) {
                icon_i = i;
                icon_c = c;
                if (icon_i <= 2) {
                    setMode(icon_c == 'n' ? NIGHT_MODE : SUNNY_MODE);
                } else if (icon_i <= 4) {
                    setMode(icon_c == 'n' ? NIGHT_MODE : CLOUDY_MODE);
                } else if (icon_i <= 9) {
                    setMode(LIGHTRAIN_MODE);
                } else if (icon_i == 10) {
                    setMode(HARDRAIN_MODE);
                } else if (icon_i == 11) {
                    setMode(LIGHTNING_MODE);
                } else {
                    setMode(icon_c == 'n' ? NIGHT_MODE : SUNNY_MODE);
                }
            }
        }

        
    public:
        Dot* peers[MATRIX_Y];
        int mode;
        int icon_i;
        char icon_c;
        
        WeatherGFX(WobblyTime* _wTime) {
            wTime = _wTime;
            for (int i = 0; i < MATRIX_Y; i++) {
                Log.trace("GFX: new bug %d", i);
                peers[i] = new WeatherBug(wTime);
            }
            icon_i = 1;
            icon_c = 'd';
            setMode(SUNNY_MODE);
        } // WeatherGFX()
        
        
        void setup() {
            #ifdef DEBUG_GFX
              Particle.variable("gfx_icon", this->icon_s);
              Particle.variable("gfx_mode", this->mode);
              Particle.variable("gfx_i", this->icon_i);
            #endif
        } // setup()


        void run(String icon) {
            Log.trace("GFX: run(%s)", icon.c_str());
            update(icon);
            Log.trace("GFX: updated()");
            bool flash = P(10);
            for (int i = 0; i < MATRIX_Y; i++) {
                Log.trace("GFX: bug %d", i);
                WeatherBug* bug = (WeatherBug*)peers[i];
                if (mode == LIGHTNING_MODE && flash) {
                    bug->set_color(WHITE);
                } else {
                    bug->run(peers);
                }
            }
        } // run(icon)
};
#endif
