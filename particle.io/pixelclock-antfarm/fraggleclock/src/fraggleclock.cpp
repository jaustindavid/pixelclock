/* 
 * Project fraggleclock
 * Author: Austin David
 * Date: 4/1/24
 * For comprehensive documentation and examples, please visit:
 * https://docs.particle.io/firmware/best-practices/firmware-template/
 */

// Include Particle Device OS APIs
#include "Particle.h"

// Let Device OS manage the connection to the Particle Cloud
SYSTEM_MODE(AUTOMATIC);

// Run the application and system concurrently in separate threads
SYSTEM_THREAD(ENABLED);

// Show system, cloud connectivity, and application logs over USB
// View logs with CLI using 'particle serial monitor --follow'
SerialLogHandler logHandler(LOG_LEVEL_INFO);

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


#include <particle-dst.h>
DST dst;

#include <SimpleTimer.h>
#include <neopixel.h>

#include "defs.h"
#include "color.h"
#include "dot.h"
#include "ant.h"
#include "turtle.h"
#include "doozer.h"
#include "gfx.h"
#include "list.h"
#include "chef.h"
#include "display.h"
#include "WobblyTime.h"

#include "pinger.h"
#include "luna.h"
#include "open_weather.h"

#define PRINTF_DEBUGGER

// IMPORTANT: Set pixel COUNT, PIN and TYPE
#define PIXEL_PIN D0
#define PIXEL_COUNT (MATRIX_X * MATRIX_Y)
#define PIXEL_TYPE WS2812B
Adafruit_NeoPixel neopixels(PIXEL_COUNT, PIXEL_PIN, PIXEL_TYPE);
Display display(&neopixels);

int Dot::next_id = 0;

#define ANT_MODE 0
#define FRAGGLE_MODE 1
#define TURTLE_MODE 2
#define DOOZER_MODE 3

uint8_t mode = ANT_MODE;
// mode = TURTLE_MODE;
// mode = FRAGGLE_MODE;

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
WeatherGFX *weatherGFX;
Dot* icon_dot;

bool show_food;
bool show_weather;


WobblyTime wTime(WT_ADDY);
Chef chef;
Pinger pinger;
Luna *luna;
OpenWeather weather(WEATHER_ADDY, 15*60*1000); // 15 minute refresh period

double luna_brite = 0.0;
int display_brite = 0;



/*
 * DST
 *
 */

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

#define NUMBER_OF_FRAGGLES 1

void make_fraggles() {
    #ifdef PRINTF_DEBUGGER
        Serial.println("Making fraggles");
    #endif
    for (int i = 0; i < NUMBER_OF_FRAGGLES; i++) {
        sandbox[i] = new Fraggle();
    }
    for (int i = NUMBER_OF_FRAGGLES; i < MAX_DOTS; i++) {
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
    #ifdef PRINTF_DEBUGGER
        Serial.println("maybe adjusting one...");
    #endif
    Dot proxy = Dot(MATRIX_X-1, MATRIX_Y-2, DARKRED);
    if (!in(&proxy, sandbox)) {
        #ifdef PRINTF_DEBUGGER
            Serial.println("adding one");
        #endif
        // bin bottom-right is empty; add
        print_list(sandbox);
        Dot* brick = activate(sandbox);
        // Serial.printf("touching brick (%d,%d)\n", brick->x, brick->y);
        brick->set_color(DARKRED);
        brick->x = proxy.x;
        brick->y = proxy.y;
        print_list(sandbox);
    }
    
    // scan the bin top row; remove any bricks
    for (int x = 0; x < MATRIX_X; x++) {
        proxy.x = x;
        proxy.y = MATRIX_Y - 3;
        Dot* brick;
        if ((brick = in(&proxy, sandbox)) && (brick->get_color() == DARKRED)) {
            // Serial.printf("removing: x=%d\n", x);
            deactivate(brick, sandbox);
        }
    }
}


void loop_fraggles() {
    static SimpleTimer second(1000);
    if (second.isExpired()) {
        maybe_adjust_one_brick();
    }
    Fraggle *fraggle;
    for (int i = 0; i < NUMBER_OF_FRAGGLES; i++) {
      fraggle = (Fraggle*)sandbox[i];
      fraggle->run(food, sandbox, bin);
    }
  }


/*
 * DOOZERS
 *
 */

#define NUMBER_OF_DOOZERS 2

void maybe_check_brick_pile(Dot* sandbox[]) {
    Dot proxy = Dot(MATRIX_X-1, MATRIX_Y-2, DARKRED);
    if (!in(&proxy, sandbox)) {
        #ifdef PRINTF_DEBUGGER
            Serial.println("adding one");
        #endif
        // print_list(sandbox);
        Dot* brick = activate(sandbox);
        // Serial.printf("touching brick (%d,%d)\n", brick->x, brick->y);
        brick->set_color(DARKRED);
        brick->x = proxy.x;
        brick->y = proxy.y;
        // print_list(sandbox);
    }

    // scan the bin top row; remove any bricks
    proxy.y = MATRIX_Y - 3;
    for (int x = 0; x < MATRIX_X; x++) {
        proxy.x = x;
        Dot* brick;
        if ((brick = in(&proxy, sandbox)) && 
            (brick->get_color() == DARKRED)) {
            // Serial.printf("removing: x=%d\n", x);
            deactivate(brick, sandbox);
        }
    }
}


void make_doozers() {
    #ifdef PRINTF_DEBUGGER
        Serial.println("Making doozers");
    #endif
    for (int i = 0; i < MAX_DOTS; i++) {
        if (i < NUMBER_OF_DOOZERS) {
            sandbox[i] = new Doozer();
            sandbox[i]->x = i;
            sandbox[i]->y = i;
        } else {
            sandbox[i] = new Dot();
        }
    }
}


void loop_doozers() {
    static SimpleTimer second(1000);
    if (second.isExpired()) {
        // maybe_adjust_one_brick();
        maybe_check_brick_pile(sandbox);
    }
    
    Doozer *doozer;
    for (int i = 0; i < NUMBER_OF_DOOZERS; i++) {
        Doozer* doozer = (Doozer*)sandbox[i];
        doozer->run(food, sandbox);
    }
}


  /*
   * TURTLE(s)
   *
   */

  #define NTURTLES 1

  void setup_turtles() {
      #ifdef PRINTF_DEBUGGER
          Serial.println("Making turtle(s)");
      #endif
      for (int i = 0; i < MAX_DOTS; i++) {
          if (i < NTURTLES) {
              sandbox[i] = new Turtle();
          } else {
              sandbox[i] = new Dot();
          }
      }
  }


  void loop_turtles() {
      for (int i = 0; i < NTURTLES; i++) {
          Turtle* turtle = (Turtle*)sandbox[i];
          turtle->run(food, sandbox);
      }
  }

  /*
   * EEPROM - burned "mode" Ants vs. Fraggles
   *
   */


  void write_mode() {
      int addy = CORE_ADDY;
      if (EEPROM.read(addy) != mode) {
          EEPROM.write(addy, mode);
      }
      addy += sizeof(mode);
      if (EEPROM.read(addy) != show_food) {
          EEPROM.write(addy, show_food);
      }
      addy += sizeof(show_food);
      if (EEPROM.read(addy) != show_weather) {
          EEPROM.write(addy, show_weather);
      }
  }


  void read_mode() {
    int addy = CORE_ADDY;
    uint8_t data = EEPROM.read(addy);
    if (data > 3) {
        // Serial.printf("Invalid data in EEPROM: 0x%02x; re-writing 'mode'\n", data);
        mode = ANT_MODE;
        write_mode();
    } else {
        mode = data;
    }
    switch (mode) {
        case TURTLE_MODE: 
            mode_name = "Turtle";
            break;
        case DOOZER_MODE:
            mode_name = "Doozer";
            break;
        case FRAGGLE_MODE:
            mode_name = "Fraggle";
            break;
        default:
            mode_name = "Ant";
            mode = ANT_MODE;
    }
    // Serial.printf("read mode: %s\n", mode_name.c_str());
    
    addy += sizeof(mode);
    show_food = EEPROM.read(addy);
    addy += sizeof(show_food);
    show_weather = EEPROM.read(addy);
}


int toggle_mode(String data) {
    mode = (mode + 1) % 4;
    write_mode();
    read_mode();
    display.clear();
    display.show();
    System.reset();
    return mode;
}


int toggle_show_food(String data) {
    show_food = !show_food;
    write_mode();
    return show_food ? 1:0;
}


int toggle_show_weather(String data) {
    show_weather = !show_weather;
    write_mode();
    return show_weather ? 1:0;
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

// dots at the bottom left (icon), right (feelslike)
void setup_weather() {
    icon_dot = new Dot(0, MATRIX_Y-1, BLACK);
    Particle.variable("icon", icon);
    weather_dot = new Dot(MATRIX_X-1, MATRIX_Y-1, BLACK);
    Particle.variable("feels_like", feels_like);
    weather.setup();
    weatherGFX = new WeatherGFX(&wTime);
    weatherGFX->setup();
}


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


/*
 * Luna
 * 
 */
void setup_luna() {
    luna = new Luna(A1, LUNA_ADDY);
    luna->setup();
}


/*
 * WiFi (backup)
 *
 */

String backupSSID;
String backupPasswd;


int set_backup_ssid(String data) {
    backupSSID = data;
    Serial.print("Saving new backup SSID: ");
    Serial.println(backupSSID);
    storeString(WIFI_ADDY, backupSSID);
    return backupSSID.length();
}


int set_backup_passwd(String data) {
    backupPasswd = data;
    Serial.print("Saving new backup passwd: ");
    Serial.println(backupPasswd);
    storeString(WIFI_ADDY+50, backupPasswd);
    return backupSSID.length();
}


void try_backup_network() {
    backupSSID = fetchString(WIFI_ADDY);
    backupPasswd = fetchString(WIFI_ADDY+50);
    Serial.println("fetched WiFi credentials:");
    Serial.println(backupSSID);
    Serial.println(backupPasswd);
    if (! WiFi.ready()) {
        Serial.printf("connecting to WiFi SSID '%s'\n", backupSSID.c_str());
        WiFi.setCredentials(backupSSID, backupPasswd);
        WiFi.connect();
    }
}


void setup_wifi() {
    Particle.function("backup_ssid", set_backup_ssid);
    Particle.function("backup_passwd", set_backup_passwd);
    try_backup_network();
}


void setup_cloud() {
    read_mode();
    Particle.variable("mode", mode_name);
    Particle.function("toggle_mode", toggle_mode);
    Particle.variable("show_food", show_food);
    Particle.function("toggle_show_food", toggle_show_food);
    Particle.function("toggle_show_weather", toggle_show_weather);
}


void setup_whatever_mode() {
    make_food();
    switch (mode) {
        case TURTLE_MODE:
            setup_turtles();
            break;
        case FRAGGLE_MODE:
            make_fraggles();
            break;
        case DOOZER_MODE:
            make_doozers();
            break;
        default:
            make_sandbox();
    }
}


void loop_whatever_mode() {
    
    switch (mode) {
        case TURTLE_MODE:
            loop_turtles();
            break;
        case FRAGGLE_MODE: 
            loop_fraggles();
            break;
        case DOOZER_MODE:
            loop_doozers();
            break;
        default:
            Ant* ant;
            for (int cursor = first(sandbox); 
                    cursor != -1; 
                    cursor = next(cursor, sandbox)) {
                ant = (Ant*)sandbox[cursor];
                ant->run(food, sandbox);
            }
    }
}


void prefill(Dot* food[], Dot* sandbox[]) {
    Dot* proxy;
    for (int i = first(food); i != -1; i = next(i, food)) {
        if (!in(food[i], sandbox)) {
            proxy = activate(sandbox);
            proxy->x = food[i]->x;
            proxy->y = food[i]->y;
            proxy->color = RED;
        }
    }
} // void prefill(Dot* food[], Dot* sandbox[])


// Global variable to hold the watchdog object pointer
ApplicationWatchdog *wd;

void watchdogHandler() {
  // Do as little as possible in this function, preferably just
  // calling System.reset().
  // Do not attempt to Particle.publish(), use Cellular.command()
  // or similar functions. You can save data to a retained variable
  // here safetly so you know the watchdog triggered when you 
  // restart.
  // In 2.0.0 and later, RESET_NO_WAIT prevents notifying the cloud of a pending reset
  System.reset(RESET_NO_WAIT);
}


void setup() {
    
    Serial.begin(115200);

    #ifdef PRINTF_DEBUGGER
        waitFor(Serial.isConnected, 30000);
        Serial.println("Beginning display.init()");
    #endif

    wd = new ApplicationWatchdog(30000, watchdogHandler, 1536);
    setup_wifi();
    
    display.setup();
    setup_dst();
    setup_cloud();
    wTime.setup();
    setup_weather();
    setup_luna();

    setup_whatever_mode();

    chef.setup();
    chef.cook(food, wTime.hour(), wTime.minute(), wTime.metric());
    // cook();
    
    prefill(food, sandbox);

    Serial.print(" initialization complete; free mem == ");
    Serial.println(System.freeMemory());
} // setup()


void loop() {
    uint32_t freemem = System.freeMemory();
    if (second.isExpired()) {
        Serial.print("free memory: ");
        Serial.println(freemem);
        chef.cook(food, wTime.hour(), wTime.minute(), wTime.metric());

        if (Particle.connected()) {
            Particle.syncTime();
            dst.check();
        }
        // Serial.printf("Sandbox has %d ants; food has %d dots\n", len(sandbox), len(food));
        luna_brite = luna->get_brightness();
        display_brite = display.set_brightness(luna_brite);
    }
    /*
    if (minute.isExpired()) {
        if (wTime.metric()) {
            Particle.publish("tick", 
                String::format("wobbly %02d.%02d, actual %02d:%02d, free %u, %u uptime", 
                            wTime.hour(), wTime.minute(), 
                            Time.hour(), Time.minute(),
                            System.freeMemory(), millis()/60000));
        } else {
            Particle.publish("tick", 
                String::format("wobbly %02d:%02d, actual %02d:%02d, free %u, %u uptime", 
                            wTime.hour(), wTime.minute(), 
                            Time.hour(), Time.minute(),
                            System.freeMemory(), millis()/60000));
        }
    }
    */
    if (daily.isExpired()) {
        if (!Particle.connected()) {
            Particle.connect();
            delay(1000); // Wait for connection attempt
        }
    }
    
    // Particle.publish("beep"); 
    // delay(10000);
    loop_whatever_mode();
    
    display.clear();
    if (show_weather) {
        update_weather();
        weatherGFX->run(weather.icon_str);
        display.render(weatherGFX->peers);
    }
    if (show_food) {
        display.render(food);
    }
    display.render(sandbox);
    display.render(pinger.pings(), pinger.npings());
    // first few things in the sandbox are always cursors
    display.render(sandbox, 2);
    display.show(show_timer);
    
    /*
    if (minute.isExpired()) {
        String str = String::format("%d %c", weatherGFX->icon_i, weatherGFX->icon_c);
        Particle.publish("weather icon", str);
    }
    */
} // loop()
