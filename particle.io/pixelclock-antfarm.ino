
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

// IMPORTANT: Set pixel COUNT, PIN and TYPE
#define PIXEL_PIN D0
#define PIXEL_COUNT (MATRIX_X * MATRIX_Y)
#define PIXEL_TYPE WS2812B
Adafruit_NeoPixel neopixels(PIXEL_COUNT, PIXEL_PIN, PIXEL_TYPE);
Display display(&neopixels);

#define FPS 5
// SimpleTimer every50(1000/10); // 10 FPS  
SimpleTimer* show_timer = new SimpleTimer(1000/4/FPS);
SimpleTimer every50(100);
SimpleTimer second(1000);
SimpleTimer daily(24*60*60*1000);
Dot* food[MAX_DOTS];
Dot* sandbox[MAX_DOTS];
void make_sandbox();

WobblyTime wTime(30, 180);
Chef chef;
Pinger pinger;
Luna *luna;


void setup() {
    Serial.begin(115200);
    #ifdef TESTING
            waitFor(Serial.isConnected, 15000);
    #endif
    
    #ifdef PRINTF_DEBUGGER
        Serial.println("Beginning display.init()");
    #endif
    display.init();
    
    setup_dst();
    luna = new Luna(A1);
    luna->get_brightness();
    
    make_food();
    make_sandbox();

    food[0]->active = true;
    food[0]->x = 0;
    food[0]->y = 0;
    food[0]->color = GREEN;

    Dot* morsel = activate(food);
    morsel->x = 1;
    morsel->y = 1;
    morsel->color = BLUE;
    
    
    Serial.printf("%d active foods\n", len(food));
    
    display.render(food);
    display.show(show_timer);
    delay(1000);
    int cursor = first(food);
    food[cursor]->color = BLUE;
    display.render(food);
    display.show(show_timer);
    delay(1000);
    for (cursor = first(food); cursor != -1; cursor = next(cursor, food)) {
        Serial.printf("cursor=%d\n", cursor);
        food[cursor]->color = YELLOW;
        display.render(food);
        display.show();
        delay(500);
    }
    chef.cook(food, wTime.hour(), wTime.minute());
    display.render(food);
    display.show();
    

    // clear_dots((Dot**)sandbox);
    Serial.printf("Sandbox has %d ants; food has %d dots\n", len(sandbox), len(food));
    Ant* ant = (Ant*)activate(sandbox);
    int i = first(sandbox);
    Serial.printf("Sandbox *now* has %d ants; food has %d dots\n", len(sandbox), len(food));
    Serial.printf("first ant: %d, color=%08x\n", i, sandbox[i]->color);
    activate(sandbox);
    i = next(i, sandbox);
    Serial.printf("next ant: %d, color=%08x\n", i, sandbox[i]->color);
} // setup()



void loop() {
    uint32_t start_ms = millis();
    if (second.isExpired()) {
        uint32_t freemem = System.freeMemory();
        Serial.print("free memory: ");
        Serial.println(freemem);
        chef.cook(food, wTime.hour(), wTime.minute());
        // display.render(food);
        if (Particle.connected()) {
            Particle.syncTime();
            dst.check();
        }
        // Serial.printf("Sandbox has %d ants; food has %d dots\n", len(sandbox), len(food));
        display.set_brightness(luna->get_brightness());
    }
    if (daily.isExpired()) {
        if (Particle.connected()) {
            Particle.syncTime();
            dst.check();
        }
    }
    for (int cursor = first(sandbox); cursor != -1; cursor = next(cursor, sandbox)) {
        Ant* ant = (Ant*)sandbox[cursor];
        // Serial.printf("ant: %d, (%d,%d), color=%08x\n", cursor, ant->x, ant->y, ant->color);
        // ant->wander(sandbox);
        ant->run(food, sandbox);
    }
    
    display.clear();
    // display.render(food);
    display.render(sandbox);
    display.render(pinger.pings());
    Serial.printf("loop duration: %d ms\n", millis() - start_ms);
    display.show(show_timer);
//    #ifdef PRINTF_DEBUGGER
        // Serial.printf("dot: (%02d,%02d):%08x\n", dot.x, dot.y, dot.color);
//    #endif
    // every50.wait();
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