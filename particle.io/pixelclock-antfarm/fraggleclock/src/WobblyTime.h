#pragma once

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
 * Exception: if Time() gets reset or sync'd this can get
 * REALLY OFF.  In that case we'll snap back to the correct
 * time
 * 
 */
 

#undef DEBUG_WT

int rando(int rmin, int rmax) {
    float r = 0.000001 * random(0, 1000000);
    return constrain(rmin + (int)(r*r*r*(rmax - rmin)), rmin, rmax);
}


class WobblyTimer {
    private:
        uint32_t _min_interval, _max_interval;
        SimpleTimer *_timer;


    public:

        WobblyTimer(uint32_t min_interval, uint32_t max_interval) {
            _min_interval = min_interval;
            _max_interval = max_interval;
            _timer = new SimpleTimer(99);
            reset();
        } // constructor
        

        bool isExpired() {
            if (_timer->isExpired()) {
                reset();
                return true;
            }
            return false;
        } // bool isExpired()
        

        void reset() {
            uint32_t interval = map(random(100000), 
                                    0, 100000, 
                                    _min_interval, _max_interval);
            Particle.publish("wobblytimer", 
                             String::format("interval: %ld", interval));
            _timer->setInterval(interval); 
            _timer->reset();
        } // reset()
        

        void publish() {
            Particle.publish("wobblytimer", 
                String::format("min: %ld, max: %ld", 
                               _min_interval, _max_interval));
        } // publish()
}; 


class WobblyTime {
    private:
        byte h, m, s, forced_hh, forced_mm;
        int address, dT, target_offset;
        int MAX_ADVANCE, MIN_ADVANCE;
        time_t fakeTime, lastTick, lockout;
        SimpleTimer *tickTimer; 
        int hhmm;

        void init(int, int);
        void update();
        int setAdvance(String);
        bool coarse_correct();
        void tick();
        void printStatus();
        int setMinAdvance(String);
        int setMaxAdvance(String);
        int setTime(String);
        void read_data();
        void write_data();

    public:
        WobblyTime(int);
        void setup_cloud();
        time_t now();
        byte hour();
        byte minute();
        byte second();
};


WobblyTime::WobblyTime(int newAddress) {
    address = newAddress;
    read_data();
    dT = rando(MIN_ADVANCE, MAX_ADVANCE);
    target_offset = rando(MIN_ADVANCE, MAX_ADVANCE);
    fakeTime = Time.now() + rando(MIN_ADVANCE, MAX_ADVANCE);
    tickTimer = new SimpleTimer(1000);
    lockout = lastTick = Time.now();
    hhmm = 0;
    printStatus();
} // init(min, max)


int WobblyTime::setAdvance(String s) {
    long t = s.toInt();
    if (t) {
        // fakeTime = max(Time.now() + t, fakeTime);
        dT = constrain(t, MIN_ADVANCE, MAX_ADVANCE);
        target_offset = constrain(t, MIN_ADVANCE, MAX_ADVANCE);
    }
    return dT;
} // int setAdvance(s)


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


void WobblyTime::setup_cloud() {
    Particle.function("wt_min_advance", &WobblyTime::setMinAdvance, this);
    Particle.function("wt_max_advance", &WobblyTime::setMaxAdvance, this);
    #ifdef DEBUG_WT
      Particle.function("wt_time", &WobblyTime::setTime, this);
      Particle.variable("wt_hhmm", this->hhmm);
      Particle.function("wt_advance", &WobblyTime::setAdvance, this);
    #endif
} // setup()


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
    if (MIN_ADVANCE == -1 
        || abs(MIN_ADVANCE) > 3600) {
        MIN_ADVANCE = 180;
    }
    a += sizeof(MIN_ADVANCE);
    EEPROM.get(a, MAX_ADVANCE);
    if (MAX_ADVANCE == -1 
        || abs(MAX_ADVANCE) > 3600) {
        MAX_ADVANCE = 300;
    }
    // a += sizeof(MAX_ADVANCE);
} // read_data()


void WobblyTime::write_data() {
    int a = address;
    EEPROM.put(a, MIN_ADVANCE);
    a += sizeof(MIN_ADVANCE);
    EEPROM.put(a, MAX_ADVANCE);
    // a += sizeof(MAX_ADVANCE);
} // write_data()


int WobblyTime::setMinAdvance(String data) {
    int value = data.toInt();
    if (value != 0) {
        MIN_ADVANCE = value;
        write_data();
    }
    return MIN_ADVANCE;
} // int setMinAdvance(data)


int WobblyTime::setMaxAdvance(String data) {
    int value = data.toInt();
    if (value != 0) {
        MAX_ADVANCE = value;
        write_data();
    }
    return MAX_ADVANCE;
} // int setMaxAdvance(data)


// RARE, but sometimes Time() can get sync'd for
// a large correction.  If this is needed, just do it
bool WobblyTime::coarse_correct() {
  int actual_offset = fakeTime - Time.now();
  if (abs(actual_offset) > 2*MAX_ADVANCE) {
    // quick slam to proper time
    Log.warn("WobblyTime massive correction: %d", actual_offset);
    fakeTime = Time.now();
  }
  return false;
} // bool coarse_correct()


void WobblyTime::update() {
    if (! coarse_correct()) {
      tick();
    }
    h = Time.hour(fakeTime);
    m = Time.minute(fakeTime);
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


void WobblyTime::printStatus() {
    Serial.printf("Wobbly Time: %02d:%02d:%02d, delta %llds; actual %02d:%02d:%02d\n", 
                    hour(), minute(), second(),
                    fakeTime - Time.now(), 
                    Time.hour(), Time.minute(), Time.second());
} // printStatus()
