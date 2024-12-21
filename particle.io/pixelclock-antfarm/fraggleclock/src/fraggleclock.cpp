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
// SYSTEM_MODE(SEMI_AUTOMATIC);

// Run the application and system concurrently in separate threads
SYSTEM_THREAD(ENABLED);

// Show system, cloud connectivity, and application logs over USB
// View logs with CLI using 'particle serial monitor --follow'
// SerialLogHandler logHandler(LOG_LEVEL_TRACE);
// SerialLogHandler logHandler(LOG_LEVEL_INFO);
SerialLogHandler logHandler(LOG_LEVEL_WARN);

/*
 * A compact implementation of the pixelclock
 *
 * NB / food for thought: there's not a GREAT reason to create/destroy objects, 
 * vs activating / deactivating static objects.
 *
 * ps tried this, it's fast as h*ck
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
#include "layout.h"
#include "color.h"
#include "dot.h"
#include "ant.h"
#include "turtle.h"
#include "doozer.h"
#include "raccoon.h"
#include "gfx.h"
#include "list.h"
#include "chef.h"
#include "display.h"
#include "WobblyTime.h"

#include "pinger.h"
#include "luna.h"
#include "open_weather.h"
#include "temperature_graph.h"

#define PRINTF_DEBUGGER


#define PHOTON2 32

#ifndef PIXEL_PIN
  #if (PLATFORM_ID == PHOTON2)
    #define PIXEL_PIN SPI
  #else // 
    #define PIXEL_PIN D0
  #endif
#endif

#define CDS_POWER  A0
#define CDS_SENSE  A1
#define CDS_GROUND A2

// IMPORTANT: Set pixel COUNT, PIN and TYPE
// #define PIXEL_COUNT 256 // moved to defs.h
#define PIXEL_TYPE WS2812B
Adafruit_NeoPixel neopixels(PIXEL_COUNT, PIXEL_PIN, PIXEL_TYPE);
Display display(&neopixels);

int Dot::next_id = 0;

#define ANT_MODE 0
#define FRAGGLE_MODE 1
#define TURTLE_MODE 2
#define DOOZER_MODE 3
#define RACCOON_MODE 4
#define MAX_MODE 5

uint8_t mode = ANT_MODE;
// mode = TURTLE_MODE;
// mode = FRAGGLE_MODE;

String mode_name = "Ant";

SimpleTimer every50(100);
SimpleTimer second(1000);
SimpleTimer minute(60*1000);
SimpleTimer hourly(60*60*1000);
SimpleTimer daily(24*60*60*1000);
Dot* food[MAX_DOTS];
Dot* sandbox[MAX_DOTS];
Dot* bin[MAX_DOTS];
WeatherGFX *weatherGFX;
TemperatureGraph *temperature_graph;
Layout layout;

bool reboot_me = false; // used for a mode switch


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
 * LAYOUT MANAGEMENT
 */

// resizes a few things based on various modes
void update_layout() {
  switch (mode) {
    case TURTLE_MODE: 
      if (layout.show_weather) {
        pinger.set_layout(1, MATRIX_Y-2);
      } else {
        pinger.set_layout(0, MATRIX_Y);
      }
      break;
    case DOOZER_MODE:
    case FRAGGLE_MODE:
      if (layout.show_weather) {
        pinger.set_layout(1, MATRIX_Y-2);
      } else {
        pinger.set_layout(0, MATRIX_Y);
      }
      break;
    case RACCOON_MODE:
      update_raccoon_layout(&layout, sandbox);
      if (layout.show_weather) {
        pinger.set_layout(2, MATRIX_Y-5);
      } else {
        pinger.set_layout(1, MATRIX_Y-3);
      }
      break;
    case ANT_MODE:
    default:
      if (layout.show_weather) {
        pinger.set_layout(1, MATRIX_Y-2);
      } else {
        pinger.set_layout(0, MATRIX_Y);
      }
  }
} // update_layout()


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
      addy += sizeof(show_weather);
  } // write_mode()


  void read_mode() {
    int addy = CORE_ADDY;
    uint8_t data = EEPROM.read(addy);
    if (data >= MAX_MODE) {
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
        case RACCOON_MODE:
            mode_name = "Raccoon";
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
} // read_mode()


// if given 0 or nonsense, just increments
int toggle_mode(String data) {
    int m = data.toInt();
    if (m > 0 && m < MAX_MODE) {
        mode = m;
    } else {
        mode = (mode + 1) % MAX_MODE;
    }
    write_mode();
    read_mode();
    reboot_me = true;
    return mode;
} // toggle_mode(s)


// waits 5s, then reboots
void maybe_reboot() {
    if (reboot_me) {
      delay(5000);
      display.clear();
      display.show();
      System.reset();
    }
} // maybe_reboot()


int toggle_show_food(String data) {
    show_food = !show_food;
    write_mode();
    return show_food ? 1:0;
} // toggle_show_food(s)


int toggle_show_weather(String data) {
    show_weather = !show_weather;
    write_mode();
    return (show_weather ? 1:0);
} // toggle_show_weateher()


/*
 * Weather
 *
 */

float feels_like() {
    return weather.feels_like();
} // feels_like()


int icon() {
    return weather.icon();
} // icon()


void setup_weather() {
    weather.setup();
    weatherGFX = new WeatherGFX(&wTime);
    weatherGFX->setup();
    temperature_graph = new TemperatureGraph(0); // x = 0
} // setup_weather()


/*
// TODO: move this out & refactor
void update_weather() {
    if (weather.feels_like() > 25) {
        weather_dot->set_color(GREEN);
    } else if (weather.feels_like() > 10) {
         weather_dot->set_color(YELLOW);
    } else {
         weather_dot->set_color(RED);
    }
    display.paint(weather_dot);
} // update_weather()
*/


/*
 * Luna
 * 
 */

void setup_luna() {
    luna = new Luna(CDS_POWER, CDS_SENSE, CDS_GROUND, LUNA_ADDY);
    luna->setup();
} // setup_luna()


/*
 * WiFi (backup) and hotspot
 *
 */



int set_backup_ssid(String data) {
    String backupSSID = data;
    Log.info("Saving new backup SSID: %s", backupSSID.c_str());
    storeString(WIFI_ADDY, backupSSID);
    return backupSSID.length();
} // set_backup_ssid(data)


int set_backup_passwd(String data) {
    String backupPasswd = data;
    Log.info("Saving new backup passwd: %s", backupPasswd.c_str());
    storeString(WIFI_ADDY+50, backupPasswd);
    return backupPasswd.length();
} // set_backup_passwd(data)


// toggles between the "backup" network and EMERGENCY network
void try_backup_network() {
    static bool emergency_creds = false;
    String backupSSID = String(WIFI_EMERGENCY_SSID);
    String backupPasswd = String(WIFI_EMERGENCY_PASSWD);

    if (! WiFi.ready()) {
        if (!emergency_creds) {
            backupSSID = fetchString(WIFI_ADDY);
            backupPasswd = fetchString(WIFI_ADDY+50);
            Log.info("fetched WiFi credentials: %s :: %s",
                backupSSID.c_str(),
                backupPasswd.c_str());
        }
        emergency_creds = !emergency_creds;
        Serial.printf("connecting to WiFi SSID '%s'\n", backupSSID.c_str());
        WiFi.setCredentials(backupSSID, backupPasswd);
        WiFi.connect();
    }
} // try_backup_network()


void paint_connection_status(int x, int y) {
  if (Particle.connected()) {
    display.paint(txlate(x, y), GREEN);
  } else if (WiFi.ready()) {
    display.paint(txlate(x, y), YELLOW);
  } else {
    display.paint(txlate(x, y), RED);
  }
} // paint_connection_status(int x, int y)


void connect_and_blink(int y, String WIFI_SSID, String WIFI_PASSWD) {
  SimpleTimer thirty_secs(30*1000);
  bool lit = true;

  paint_connection_status(0, y);
  if (WIFI_SSID.length() != 0) {
    Log.warn("Setting new WiFi creds: %s :: %s", 
              WIFI_SSID.c_str(), WIFI_PASSWD.c_str());
    WiFi.setCredentials(WIFI_SSID, WIFI_PASSWD);
  }
  Log.warn("starting Particle.connect()");
  Particle.connect();
  while (!thirty_secs.isExpired()
         && ! Particle.connected()) {
    if (lit) {
      paint_connection_status(0, y);
    } else {
      display.paint(txlate(0, y), BLACK);
    }
    display.show();
    delay(1000);
    ApplicationWatchdog::checkin(); // resets the AWDT count
  }

  paint_connection_status(0, y);
  display.show();
} // connect_and_blink(y, WIFI_SSID, WIFI_PASSWD)


// will try main, backup, and emergency networks
void try_to_connect() {
  if (Particle.connected()) {
    return;
  }

  ApplicationWatchdog::checkin(); // resets the AWDT count

  Log.warn("trying default credentials...");
  connect_and_blink(0, String(""), String(""));
  if (Particle.connected()) {
    Log.warn("Connected!");
    return;
  }

  ApplicationWatchdog::checkin(); // resets the AWDT count

  String backupSSID = fetchString(WIFI_ADDY);
  String backupPasswd = fetchString(WIFI_ADDY+50);
  Log.warn("trying backup credentials %s :: %s", 
      backupSSID.c_str(), backupPasswd.c_str());
  connect_and_blink(1, backupSSID, backupPasswd);
  if (Particle.connected()) {
    Log.warn("Connected to backup!");
    return;
  }

  ApplicationWatchdog::checkin(); // resets the AWDT count

  Log.warn("trying emergency credentials %s :: %s", 
            WIFI_EMERGENCY_SSID, WIFI_EMERGENCY_PASSWD);
  connect_and_blink(2, WIFI_EMERGENCY_SSID, WIFI_EMERGENCY_PASSWD);
  if (Particle.connected()) {
    Log.warn("Connected to emergency hotspot!");
    return;
  }

  ApplicationWatchdog::checkin(); // resets the AWDT count
  Log.warn("Failed to connect :(((");
} // try_to_connect()


void setup_wifi() {
    uint32_t start_time = millis();
    Particle.function("backup_ssid", set_backup_ssid);
    Particle.function("backup_passwd", set_backup_passwd);

    try_to_connect();

    // wait for the remainder of HOLDING_PATTERN seconds
    while (millis() - start_time < HOLDING_PATTERN * 1000) {
      display.paint(txlate(1, 0), BLUE);
      display.show();
      delay(500);
      display.paint(txlate(1, 0), BLACK);
      display.show();
      delay(500);
    }
} // setup_wifi()


void setup_cloud() {
  static bool already_set = false;

  if (! already_set 
      && Particle.connected()) {
    read_mode();
    Particle.variable("mode", mode_name);
    Particle.function("toggle_mode", toggle_mode);
    already_set = true;
  }  
} // setup_cloud()


void setup_whatever_mode() {
    make_food();
    switch (mode) {
        case TURTLE_MODE:
            setup_turtles(sandbox);
            break;
        case FRAGGLE_MODE:
            setup_fraggles(sandbox);
            break;
        case DOOZER_MODE:
            setup_doozers(sandbox);
            break;
        case RACCOON_MODE:
            make_raccoons(sandbox);
            break;
        default:
            make_sandbox();
    }
} // setup_whatever_mode()


void loop_whatever_mode() {
    // Log.trace("loop_whatever_mode: mode=%d", mode);
    // delay(1000);
    
    switch (mode) {
        case TURTLE_MODE:
            loop_turtles(food, sandbox);
            break;
        case FRAGGLE_MODE: 
            loop_fraggles(food, sandbox);
            break;
        case DOOZER_MODE:
            loop_doozers(food, sandbox);
            break;
        case RACCOON_MODE:
            loop_raccoons(food, sandbox);
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
} // loop_whatever_mode()


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
} // watchdogHandler()


void maybe_reconnect() {
  if (!Particle.connected()) {
      WiFi.off();
      delay(30*1000);
      WiFi.on(); 
      delay(30*1000);
      Particle.connect();
      delay(10000); // Wait for connection attempt
  }

  if (Particle.connected()) {
      Particle.syncTime();
      dst.check();
  }
} // maybe_reconnect()


void setup() {
    Serial.begin(115200);

    display.setup();

    waitFor(Serial.isConnected, 10000);
    setup_wifi(); // takes at least 30s
                  
    wd = new ApplicationWatchdog(30000, watchdogHandler, 1536);

    setup_dst();
    setup_cloud();
    layout.setup();
    setup_color();
    wTime.setup();
    setup_weather();
    setup_luna();
    pinger.setup();

    setup_whatever_mode();

    chef.setup();
    chef.cook(food, wTime);
    // cook();
    
    // prefill(food, sandbox);

    Log.warn("initialization complete; free mem == %ld", System.freeMemory());
    //display.test_forever();
} // setup()


void loop() {
    if (second.isExpired()) {
        Log.info("free memory: %ld", System.freeMemory());
        chef.cook(food, wTime);
        // Log.trace("loop s1");

        // Serial.printf("Sandbox has %d ants; food has %d dots\n", len(sandbox), len(food));

        luna_brite = luna->get_brightness();
        // Log.trace("loop s2; luna_brite = %5.2f", luna_brite);
        display_brite = display.set_brightness(luna_brite);
        // Log.trace("loop s3; display_brite = %d", display_brite);
        // delay(1000);
    }
    if (hourly.isExpired()) {
        Log.trace("loop d1");
        maybe_reconnect();
    }
    
    // Log.trace("loop 2");
    loop_whatever_mode();
    
    display.clear();

    update_layout();
    
    if (layout.show_weather) {
        // update_weather();
        weatherGFX->run(weather.icon_str);
        // display.render(weatherGFX->peers);
        display.render(weatherGFX->peers, MATRIX_Y);
        temperature_graph->update(weather.feels_like());
        display.render(temperature_graph->dots, MATRIX_Y);
    }
    
    if (layout.show_plan) {
        display.render(food);
    }

    if (layout.show_pinger) {
      display.render(pinger.pings(), pinger.npings());
    }
    display.render(sandbox);
    display.show_multipass();
    maybe_reboot();
} // loop()
