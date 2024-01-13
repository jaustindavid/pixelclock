/*
 * A compact implementation of the pixelclock
 *
 * NB / food for thought: there's not a GREAT reason to create/destroy objects, 
 * vs activating / deactivating static objects.
 *
 * ps tried this, it'as fast as h*ck
 * 
 * dot: 
 *   a sprite
 *   equals(other)
 *   active/inactive (nee new/delete)
 *
 * food:    list (array) of dots
 * sandbox: list (array) of ants
 *
 * ant: a dot which...
 *   avoids other ants
 *   nop: stay on food (nop)
 *   seek: walk toward food
 *   wander
 *
 * queen: an ant which ...
 *   if not enough ants: make one
 *   if too many ants: eat one
 *   wander
 *
 * primitives needed:
 *   contents of cell at location (is anything in list at this location)
 *   location of "nearest thing" (thing in list sort-of near me)
 *   any <thing> adjacent 
 *   distance / vector to <thing>
 *
 * Fraggle: an ant which *moves* Dots (bricks) into food locations
 *   Chef: the same.  "food" still a thing
 *   Sandbox: pre-filled with a few fraggles and mostly bricks (red Dots).
 */


#include "Particle.h"

#include <particle-dst.h>
DST dst;

#include <SimpleTimer.h>
#include <neopixel.h>

#include "defs.h"
#include "color.h"
#include "dot.h"
#include "ant.h"
#include "list.h"
#include "display.h"
#include "WobblyTime.h"

#include "chef.h"
#include "pinger.h"
#include "luna.h"
#include "open_weather.h"

// IMPORTANT: Set pixel COUNT, PIN and TYPE
#define PIXEL_PIN D0
#define PIXEL_COUNT (MATRIX_X * MATRIX_Y)
#define PIXEL_TYPE WS2812B
Adafruit_NeoPixel neopixels(PIXEL_COUNT, PIXEL_PIN, PIXEL_TYPE);
Display display(&neopixels);


#define MODE_STORAGE 0
#define ANT_MODE 0
#define FRAGGLE_MODE 1
uint8_t mode = ANT_MODE;
String mode_name = "Ant";


#define FPS 5
// SimpleTimer every50(1000/10); // 10 FPS  
SimpleTimer* show_timer = new SimpleTimer(1000/4/FPS);
SimpleTimer every50(100);
SimpleTimer second(1000);
SimpleTimer minute(60*1000);
SimpleTimer daily(24*60*60*1000);
Dot* food[MAX_DOTS];
Dot* sandbox[MAX_DOTS];
Dot* bin[MAX_DOTS];
Dot* weather_dot;
Dot* icon_dot;

WobblyTime wTime(30, 180);
Chef chef;
int forced_hh = -1, forced_mm = -1;
Pinger pinger;
Luna *luna;
OpenWeather weather(60*1000); // 15 minute refresh period


void setup() {
    Serial.begin(115200);
    waitFor(Serial.isConnected, 15000);

    #ifdef PRINTF_DEBUGGER
        Serial.println("Beginning display.init()");
    #endif
    display.init();
    
    setup_dst();
    setup_cloud();
    setup_weather();
    
    luna = new Luna(A1);

    read_mode();

    if (mode == ANT_MODE) {
        make_food();
        make_sandbox();
    } else {
        make_food();
        make_fraggles();
    }
    chef.cook(food, wTime.hour(), wTime.minute());
    // cook();
    
    Serial.print(mode == ANT_MODE ? "Ant" : "Fraggle");
    Serial.print(" initialization complete; free mem == ");
    Serial.println(System.freeMemory());
    delay(10000);
} // setup()



void loop() {
    uint32_t start_ms = millis();
    if (second.isExpired()) {
        uint32_t freemem = System.freeMemory();
        Serial.print("free memory: ");
        Serial.println(freemem);
        if (forced_hh == -1) {
            chef.cook(food, wTime.hour(), wTime.minute());
        } else {
            chef.cook(food, forced_hh, forced_mm);
            forced_hh = forced_mm = -1;
        }

        if (Particle.connected()) {
            Particle.syncTime();
            dst.check();
        }
        // Serial.printf("Sandbox has %d ants; food has %d dots\n", len(sandbox), len(food));
        display.set_brightness(luna->get_brightness());
        if (mode == FRAGGLE_MODE) {
            maybe_adjust_one_brick();
        }
    }
    if (daily.isExpired()) {
        if (!Particle.connected()) {
            Particle.connect();
            delay(1000); // Wait for connection attempt
        }
    }
    if (mode == ANT_MODE) {
        for (int cursor = first(sandbox); cursor != -1; cursor = next(cursor, sandbox)) {
            Ant* ant = (Ant*)sandbox[cursor];
            ant->run(food, sandbox);
        }
    } else {
        Fraggle* fraggle = (Fraggle*)sandbox[0];
        fraggle->run(food, sandbox, bin);
        fraggle = (Fraggle*)sandbox[1];
        fraggle->run(food, sandbox, bin);
    }
    
    display.clear();
    update_weather();
    display.render(food);
    display.render(sandbox);
    display.render(pinger.pings());
    // Serial.printf("loop duration: %d ms\n", millis() - start_ms);
    display.show(show_timer);
} // loop()


void setup_dst() {
    dst_limit_t beginning;
    dst_limit_t end;
    Particle.syncTime();
    waitUntil(Particle.syncTimeDone);

    Time.zone(-5);

    beginning.hour = 2;
    beginning.day = DST::days::sun;
    beginning.month = DST::months::mar;
    beginning.occurrence = 2;
        
    end.hour = 2;
    end.day = DST::days::sun;
    end.month = DST::months::nov;
    end.occurrence = 1;
        
    dst.begin(beginning, end, 1);
    dst.automatic(true);
    dst.check();
} // setup_dst()


void setup_cloud() {
    Particle.variable("mode", mode_name);
    Particle.function("toggle_mode", toggle_mode);
    Particle.function("set_time", set_time);
}


// sets a time hh:mm; for debugging,
// the time will get displayed then immediately corrected
int set_time(String hhmm) {
    if (hhmm.length() == 5) {
        String hh = hhmm.substring(0, 2);
        forced_hh = hh.toInt();
        String mm = hhmm.substring(3, 5);
        forced_mm = mm.toInt();
        return forced_hh * 100 + forced_mm;
    } else {
        return -1;
    }
} // int set_time(hhmm)


/*****
 * 
 *  ANTS
 * 
 *****/
void make_food() {
    for (int i = 0; i < MAX_DOTS; i++) {
        food[i] = new Dot();
    }
}


void make_sandbox() {
    sandbox[0] = new Queen();
    for (int i = 1; i < MAX_DOTS; i++) {
        sandbox[i] = new Ant();
    }
}


/*
 * FRAGGLES
 *
 */


void make_fraggles() {
    sandbox[0] = new Fraggle();
    sandbox[1] = new Fraggle();
    for (int i = 2; i < MAX_DOTS; i++) {
        sandbox[i] = new Dot();
    }
    
    for (int i = 0; i < MAX_DOTS; i++) {
        bin[i] = new Dot();
    }
    
    // create the bin in first->last priority order
    for (int j = MATRIX_Y-2; j >= MATRIX_Y-3; j--) {
        for (int i = MATRIX_X - 1; i >= 0; i--) {
            Dot* dot = activate(bin);
            dot->x = i;
            dot->y = j;
        }
    }
}


void maybe_adjust_one_brick() {
    Dot proxy = Dot(MATRIX_X-1, MATRIX_Y-2, DARKRED);
    if (!in(&proxy, sandbox)) {
        // bin is empty; add
        Dot* brick = activate(sandbox);
        brick->set_color(DARKRED);
        brick->x = proxy.x;
        brick->y = proxy.y;
    }
    
    proxy.x = 0;
    proxy.y = MATRIX_Y - 3;
    if (in(&proxy, sandbox)) {
        // bin is full; remove
        deactivate(&proxy, sandbox);
    }
}


/*
 * EEPROM - burned "mode" Ants vs. Fraggles
 *
 */


void read_mode() {
    uint8_t data = EEPROM.read(MODE_STORAGE);
    if (data > 1) {
        Serial.printf("Invalid data in EEPROM: 0x%02x; re-writing 'mode'\n", data);
        mode = ANT_MODE;
        write_mode();
    } else {
        mode = data;
    }
    mode_name = mode ? "Fraggle" : "Ant";
}


void write_mode() {
    EEPROM.write(MODE_STORAGE, mode);
}


int toggle_mode(String data) {
    if (mode == ANT_MODE) {
        mode = FRAGGLE_MODE;
    } else {
        mode = ANT_MODE;
    }
    write_mode();
    read_mode();
    display.clear();
    display.show();
    System.reset();
    return mode;
}


/*
 * Weather
 *
 */

float feels_like() {
    return weather.feels_like();
}

int icon() {
    return weather.icon();
}

void setup_weather() {
    icon_dot = new Dot(0, 0, BLACK);
    weather_dot = new Dot(MATRIX_X-1, 0, BLACK);
    Particle.variable("feels_like", feels_like);
    Particle.variable("icon", icon);
}


SimpleTimer feelie(60*1000);
void update_weather() {
    if (weather.feels_like() > 25) {
        weather_dot->set_color(GREEN);
    } else if (weather.feels_like() > 10) {
         weather_dot->set_color(YELLOW);
    } else {
         weather_dot->set_color(RED);
    }
    display.paint(weather_dot);
    
    int i = weather.icon();
    switch(i) {             // https://openweathermap.org/weather-conditions
        case 1: icon_dot->set_color(YELLOW); // clear
            break;
        case 2:                             // few clouds
        case 3:                             // scattered clouds
        case 4:                             // broken clouds 
            icon_dot->set_color(LIGHTGREY); 
            break;
        case 9:                             // shower rain
        case 10:                            // rain
        case 11:                            // thunderstorm
            icon_dot->set_color(BLUE); 
            break;
        default:                            // mist or snow
            icon_dot->set_color(LIGHTBLUE);
    }
    display.paint(icon_dot);
}