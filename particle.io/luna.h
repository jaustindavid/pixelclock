#ifndef LUNA_H
#define LUNA_H

#include <SimpleTimer.h>

#define LOWER_LIGHT 4050  // analog value at lowest light level
#define UPPER_LIGHT 1600  // "" "" highest ""
#define LOWER_BRIGHT 0 // lowest brightness to return
#define UPPER_BRIGHT 99 // highest brightness to return

#undef PRINTF_DEBUGGER

class Luna {
    private:
        int pin;
        int last_read;
        int brightness;
        SimpleTimer *hysteresis;
        
    public:
        Luna(int);
        ~Luna() { delete hysteresis;}
        void init();
        int get_brightness();
};


Luna::Luna(int new_pin) {
    pin = new_pin;
    pinMode(pin, INPUT);
    hysteresis = new SimpleTimer(5*1000);
    last_read = analogRead(pin);
    brightness = 50;
    Particle.variable("luna_raw", last_read);
    Particle.variable("luna_brightness", brightness);
}

// returns [0..99]
int Luna::get_brightness() {
    int v = analogRead(pin);
    #ifdef PRINTF_DEBUGGER
        Serial.printf("luna: v=%d, last=%d\n", v, last_read);
    #endif
    if (hysteresis->isExpired() && 1.0*v/last_read > 0.1) { 
        #ifdef PRINTF_DEBUGGER
            Serial.println("updating last_read");
        #endif
        last_read = v;
        brightness = map(last_read, LOWER_LIGHT, UPPER_LIGHT, LOWER_BRIGHT, UPPER_BRIGHT);
    }
    return brightness;
}

#endif