#ifndef WOBBLYTIME_H
#define WOBBLYTIME_H

#include <SimpleTimer.h>

/* 
 * full disclosure: I read once about a clock which displayed imprecise time.
 * I am modeling the behavior, but I have not attempted to go back and find 
 * the author or make attribution.
 *
 * concept: display imprecise time, but always a lil fast
 *    hour() >= Time.hour()
 *    minute() >= Time.minute()
 *    second() >= Time.second()
 *
 * values will be strictly increasing; subsequent calls to second() will
 * never go backward (except for rollover obviously).  But it might appear to
 * stall, depending on settings.
 *
 * internal Theory of Operation:
 *   "target_offset" is a *goal* 
 *   actual offset = fakeTime - Time.now()
 *
 * if actual offset < target offset, 
 *   speed up time: tick faster (1, 2, or 3s ticks per s)
 * else if actual offset > target offset,
 *   slow down time: tick slower (33% chance tick per s)
 * else recalculate a target offset
 * 
 * TODO: add a few particle functions for console interaction
 */
 
#define CHAOS_NONE     0
#define CHAOS_AMPM     1
#define CHAOS_OVERFLOW 2
#define CHAOS_METRIC   3
#define NCHAOS         4


#undef DEBUG_WT

int rando(int rmin, int rmax) {
    float r = 0.000001 * random(0, 1000000);
    return rmin + (int)(r*r*r*(rmax - rmin));
}


class WobblyTimer {
    private:
        uint32_t _min_interval, _max_interval;
        SimpleTimer *_timer;
        
        void reset_timer() {
            uint32_t interval = map(random(100000), 0, 100000, _min_interval, _max_interval);
            Particle.publish("wobblytimer", String::format("interval: %5.2f h", interval/(3600*1000)));
            _timer->setInterval(interval); 
            _timer->reset();
        }
        

    public:
        WobblyTimer(uint32_t min_interval, uint32_t max_interval) {
            _timer = new SimpleTimer(99);
            reset_timer();
        }
        
        bool isExpired() {
            if (_timer->isExpired()) {
                reset_timer();
                return true;
            }
            return false;
        }
}; 


class WobblyTime {
    private:
        byte h, m, s, forced_hh, forced_mm;
        int address, MAX_ADVANCE, MIN_ADVANCE, dT, target_offset;
        time_t fakeTime, lastTick, lockout;
        SimpleTimer *tickTimer; 
        WobblyTimer *chaotic_timer;
        bool enableMetricTime, displayMetricTime;
        int chaos_mode;
        byte max_chaos;
        int hhmm;

        void init(int, int);
        void update();
        int setAdvance(String);
        void updateAdvance();
        void tick();
        void t1ck();
        void printStatus();
        int setMinAdvance(String);
        int setMaxAdvance(String);
        int setTime(String);
        void read_data();
        void write_data();
        int sync(String);
        int toggleMetricTime(String);
        int setMaxChaos(String);
        int be_chaotic();

    public:
        WobblyTime(int);
        void setup();
        time_t now();
        byte hour();
        byte minute();
        byte second();
        bool metric();
};


WobblyTime::WobblyTime(int newAddress) {
    address = newAddress;
    chaos_mode = max_chaos = 0;
    enableMetricTime = true;
    displayMetricTime = false;
    read_data();
    dT = rando(MIN_ADVANCE, MAX_ADVANCE);
    target_offset = rando(MIN_ADVANCE, MAX_ADVANCE);
    fakeTime = Time.now() + rando(MIN_ADVANCE, MAX_ADVANCE);
    tickTimer = new SimpleTimer(1000);
    chaotic_timer = new WobblyTimer(24*60*60*1000, 7*24*60*60*1000); // 1-7 days
    lockout = lastTick = Time.now();
    hhmm = 0;
    printStatus();
} // init(min, max)


/*
 *   50%: no chaos
 *   25%: AM <-> PM
 *   15%: overflow
 *   rest: metric time
 */
int WobblyTime::be_chaotic() {
    // only check at top of hour
    if (Time.minute() == 0 && chaotic_timer->isExpired()) {
        int p = random(100);
        if (p > 50) {
            return CHAOS_NONE;
        } else if (p > 25 && max_chaos >= CHAOS_AMPM) {
            return CHAOS_AMPM;
        } else if (p > 10 && max_chaos >= CHAOS_OVERFLOW) {
            return CHAOS_OVERFLOW;
        } else if (max_chaos >= CHAOS_METRIC) {
            return CHAOS_METRIC;
        }
    }
    return chaos_mode;
}


int WobblyTime::setAdvance(String s) {
    long t = s.toInt();
    if (t) {
        // fakeTime = max(Time.now() + t, fakeTime);
        dT = constrain(t, MIN_ADVANCE, MAX_ADVANCE);
        target_offset = constrain(t, MIN_ADVANCE, MAX_ADVANCE);
    }
    return fakeTime - Time.now();
}


// sets a time hh:mm; for debugging,
int WobblyTime::setTime(String hhmm) {
    if (hhmm.length() == 5) {
        String hh = hhmm.substring(0, 2);
        forced_hh = hh.toInt();
        String mm = hhmm.substring(3, 5);
        forced_mm = mm.toInt();
        lockout = Time.now() + 30;
        return forced_hh * 100 + forced_mm;
    } else {
        return -1;
    }
} // int set_time(hhmm)


void WobblyTime::setup() {
    Particle.function("wt_advance", &WobblyTime::setAdvance, this);
    Particle.function("wt_min_advance", &WobblyTime::setMinAdvance, this);
    Particle.function("wt_max_advance", &WobblyTime::setMaxAdvance, this);
    Particle.function("wt_time", &WobblyTime::setTime, this);
    Particle.variable("wt_hhmm", this->hhmm);
    // Particle.function("wt_sync", &WobblyTime::sync, this);
    Particle.function("wt_chaos", &WobblyTime::setMaxChaos, this);
    Particle.variable("wt_chaosmode", this->chaos_mode);
} // setup()


int WobblyTime::sync(String) {
    if (Particle.connected()) {
        Particle.syncTime();
        return 1;
    }
    return 0;
} // sync()


// advance up to 3x real time
void WobblyTime::tick() {
    if (tickTimer->isExpired()) {
        int lapsed = Time.now() - lastTick;
        int actual_offset = fakeTime - Time.now();
        #ifdef DEBUG_WT
            Serial.printf("actual offset: %d <> target offset %d; ", actual_offset, target_offset);
        #endif
        int tick;
        // shortcut: if the difference is less than the time lapsed, recompute
        if (abs(target_offset - actual_offset) <= lapsed) {
            #ifdef DEBUG_WT
                Serial.print("!! ");
            #endif
            tick = abs(target_offset - actual_offset);
            target_offset = rando(MIN_ADVANCE, MAX_ADVANCE);
        } else if (actual_offset < target_offset) {
            #ifdef DEBUG_WT
                Serial.print("^ ");
            #endif
            // tick 300% of realtime, but not more than the diffference; plus the ~second gap
            tick = 1 + min(target_offset - actual_offset, round(0.01*rando(lapsed*133, lapsed*300)));
        } else { //  if (actual_offset > target_offset) {
            #ifdef DEBUG_WT
                Serial.print("v ");
            #endif
            // tick 33% of realtime; ignore that 1s
            tick = round(0.01*rando(lapsed*33, lapsed*90));
        } // else {
        // }
        #ifdef DEBUG_WT
            Serial.printf("%d lapsed; ticking %d\n", lapsed, tick);
        #endif
        fakeTime += tick;
        lastTick = Time.now();
    }
} // tick()


void WobblyTime::read_data() {
    int a = address;
    EEPROM.get(a, MIN_ADVANCE);
    if (abs(MIN_ADVANCE) > 3600) {
        MIN_ADVANCE = 180;
    }
    a += sizeof(MIN_ADVANCE);
    EEPROM.get(a, MAX_ADVANCE);
    if (abs(MAX_ADVANCE) > 3600) {
        MAX_ADVANCE = 300;
    }
    a += sizeof(MAX_ADVANCE);
    EEPROM.get(a, max_chaos);
    if (max_chaos < 0 || max_chaos >= NCHAOS) {
        max_chaos = 0;
    }
}


void WobblyTime::write_data() {
    int a = address;
    EEPROM.put(a, MIN_ADVANCE);
    a += sizeof(MIN_ADVANCE);
    EEPROM.put(a, MAX_ADVANCE);
    a += sizeof(MAX_ADVANCE);
    EEPROM.put(a, max_chaos);
}


int WobblyTime::setMinAdvance(String data) {
    int value = data.toInt();
    if (value != 0) {
        MIN_ADVANCE = value;
        write_data();
    }
    return MIN_ADVANCE;
}


int WobblyTime::setMaxAdvance(String data) {
    int value = data.toInt();
    if (value != 0) {
        MAX_ADVANCE = value;
        write_data();
    }
    return MAX_ADVANCE;
}


int WobblyTime::setMaxChaos(String s) {
    long t = s.toInt();
    if (t) {
        chaos_mode = max_chaos = t % NCHAOS;
        write_data();
    } 
    return max_chaos;
} // setMaxChaos


void WobblyTime::update() {
    tick();
    // updateAdvance();
    h = Time.hour(fakeTime);
    m = Time.minute(fakeTime);

    chaos_mode = be_chaotic();
    switch (chaos_mode) {
        case CHAOS_AMPM:
            if (h < 12) {
                h += 12;
            } else {
                h -= 12;
            }
            break;
        case CHAOS_OVERFLOW:
            if (m < 10) {
                m += 60;
                if (h>0) {
                    h -= 1;
                } else {
                    h = 23;
                }
            }
            break;
        case CHAOS_METRIC:
            m = map(m, 0, 59, 0, 99);
            break;
    }

    s = Time.second(fakeTime);
    hhmm = 100 * h + m;
} // update()


time_t WobblyTime::now() {
    update();
    return fakeTime;
} // time_t now()


byte WobblyTime::hour() {
    update();
    if (Time.now() < lockout) {
        return forced_hh;
    }
    return h;
} // byte hour()


byte WobblyTime::minute() {
    update();
    if (Time.now() < lockout) {
        return forced_mm;
    }
    return m;
} // byte minute()


byte WobblyTime::second() {
    update();
    return s;
} // byte second()


bool WobblyTime::metric() {
    return chaos_mode == CHAOS_METRIC;
}


void WobblyTime::printStatus() {
    Serial.printf("Wobbly Time: %02d:%02d:%02d, delta %llds; actual %02d:%02d:%02d\n", 
                    hour(), minute(), second(),
                    fakeTime - Time.now(), 
                    Time.hour(), Time.minute(), Time.second());
}

#endif
