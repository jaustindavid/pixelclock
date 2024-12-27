/* 
 * Project fraggleclock
 * Author: Austin David
 * Date: 4/1/24
 * For comprehensive documentation and examples, please visit:
 * https://docs.particle.io/firmware/best-practices/firmware-template/
 */

// Include Particle Device OS APIs
#include "Particle.h"

// Show system, cloud connectivity, and application logs over USB
// View logs with CLI using 'particle serial monitor --follow'
// SerialLogHandler logHandler(LOG_LEVEL_TRACE);
// SerialLogHandler logHandler(LOG_LEVEL_INFO);
SerialLogHandler logHandler(LOG_LEVEL_WARN);

// #include "safer-cloud.h"
SYSTEM_MODE(AUTOMATIC);
SYSTEM_THREAD(ENABLED);

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


#include <particle-dst.h>
DST dst;

#include <SimpleTimer.h>
#include <neopixel.h>

#include "defs.h"
#include "stats.h"
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

uint8_t reboot_timer = 0; // when to reboot, in a mode switch
double uptime_h;


WobblyTime wTime(WT_ADDY);
Chef chef;
Pinger pinger;
Luna *luna;
OpenWeather weather(WEATHER_ADDY, 15*60*1000); // 15 minute refresh period

double luna_brite = 0.0;
int display_brite = 0;


/*
 * nitetime
 *
 * 99:99 - 99:99 : disable
 * any other:
 *   start nitetime when hh:mm
 *   end nitetime when hh:mm
 */

typedef struct {
  byte hh;
  byte mm;
} hh_mm_struct_t;

hh_mm_struct_t start_nitetime, end_nitetime;
String nitetime_s;
bool is_nitetime;


bool valid_hhmm(hh_mm_struct_t sample) {
  return sample.hh >= 0
      && sample.hh <= 23
      && sample.mm >= 0
      && sample.mm <= 59;
}


// ONLY WORKS ON THE BOUNDARY
// if no valid schedule, is_nitetime -> false
// otherwise, only change is_nitetime if the times match
void check_nitetime(byte hh, byte mm) {
  if (valid_hhmm(start_nitetime) 
      && valid_hhmm(end_nitetime)) {
    if (start_nitetime.hh == hh 
        && start_nitetime.mm == mm) {
      is_nitetime = true;
    }
    if (end_nitetime.hh == hh 
        && end_nitetime.mm == mm) {
      is_nitetime = false;
    }
  } else {
    is_nitetime = false;
  }
}


void stringify_nitetime() {
  if (valid_hhmm(start_nitetime) 
      && valid_hhmm(end_nitetime)) {
    nitetime_s = String::format("nitetime: %02d:%02d - %02d:%02d",
                                start_nitetime.hh, start_nitetime.mm,
                                end_nitetime.hh, end_nitetime.mm);
  } else {
    nitetime_s = "no nitetime schedule";
  }
} // stringify_nitetime()


// attempts to parse 
hh_mm_struct_t parse_hhmm(String s) {
  hh_mm_struct_t ret = { .hh = 99, .mm = 99 };
  int i = s.indexOf(":");
  if (i != 0) {
    byte hh = s.toInt();
    s = s.substring(i+1);
    byte mm = s.toInt();
    ret.hh = hh;
    ret.mm = mm;
  }
  return ret;
}


// parses "hh:mm - hh:mm"
int set_nitetime(String s) {
  hh_mm_struct_t start, end;
  start = parse_hhmm(s);
  int i = s.indexOf(" - ");
  s = s.substring(i+3);
  end = parse_hhmm(s);
  if (valid_hhmm(start) 
      && valid_hhmm(end)) {
    start_nitetime = start;
    end_nitetime = end;
    stringify_nitetime();
    return 1;
  }
  return -1;
} // int set_nitetime(s)


void setup_nitetime() {
  start_nitetime = { .hh = 99, .mm = 99 };
  end_nitetime = { .hh = 99, .mm = 99 };
  stringify_nitetime();
  is_nitetime = true;
  Particle.function("set_nitetime", set_nitetime);
  Particle.variable("nitetime_schedule", nitetime_s);
}


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
void maybe_update_layout() {
  static bool 
    old_weather = false, 
    old_pinger  = false;

  if (layout.show_weather == old_weather 
      && layout.show_pinger == old_pinger) {
    // nothing to do
    return;
  }
  old_weather = layout.show_weather;
  old_pinger = layout.show_pinger;
  switch (mode) {
    case TURTLE_MODE: 
      if (layout.show_weather) {
        pinger.set_layout(1, MATRIX_X-2);
      } else {
        pinger.set_layout(0, MATRIX_X);
      }
      break;
    case DOOZER_MODE:
    case FRAGGLE_MODE:
      update_doozer_layout(&layout, sandbox);
      if (layout.show_weather) {
        pinger.set_layout(1, MATRIX_X-2);
      } else {
        pinger.set_layout(0, MATRIX_X);
      }
      break;
    case RACCOON_MODE:
      update_raccoon_layout(&layout, sandbox);
      if (layout.show_weather) {
        pinger.set_layout(2, MATRIX_X-5);
      } else {
        pinger.set_layout(1, MATRIX_X-3);
      }
      break;
    case ANT_MODE:
    default:
      if (layout.show_weather) {
        pinger.set_layout(1, MATRIX_X-2);
      } else {
        pinger.set_layout(0, MATRIX_X);
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
    reboot_timer = 10; // seconds
    return mode;
} // toggle_mode(s)


// if reboot_timer is set:
//    decrement
//    reboot @ 0
void maybe_reboot() {
    if (reboot_timer) {
      --reboot_timer;
      if (reboot_timer == 0) {
        display.clear();
        display.show();
        System.reset();
      }
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
    #ifdef WATCHDOG_INTERVAL
      ApplicationWatchdog::checkin(); // resets the AWDT count
    #endif
  }

  paint_connection_status(0, y);
  display.show();
} // connect_and_blink(y, WIFI_SSID, WIFI_PASSWD)


// will try main, backup, and emergency networks
void try_to_connect() {
  if (Particle.connected()) {
    return;
  }

  #ifdef WATCHDOG_INTERVAL
    ApplicationWatchdog::checkin(); // resets the AWDT count
  #endif

  Log.warn("trying default credentials...");
  connect_and_blink(0, String(""), String(""));
  if (Particle.connected()) {
    Log.warn("Connected!");
    return;
  }
 
  #ifdef WATCHDOG_INTERVAL
    ApplicationWatchdog::checkin(); // resets the AWDT count
  #endif

  String backupSSID = fetchString(WIFI_ADDY);
  String backupPasswd = fetchString(WIFI_ADDY+50);
  Log.warn("trying backup credentials %s :: %s", 
      backupSSID.c_str(), backupPasswd.c_str());
  connect_and_blink(1, backupSSID, backupPasswd);
  if (Particle.connected()) {
    Log.warn("Connected to backup!");
    return;
  }

  #ifdef WATCHDOG_INTERVAL
    ApplicationWatchdog::checkin(); // resets the AWDT count
  #endif

  Log.warn("trying emergency credentials %s :: %s", 
            WIFI_EMERGENCY_SSID, WIFI_EMERGENCY_PASSWD);
  connect_and_blink(2, WIFI_EMERGENCY_SSID, WIFI_EMERGENCY_PASSWD);
  if (Particle.connected()) {
    Log.warn("Connected to emergency hotspot!");
    return;
  }

  #ifdef WATCHDOG_INTERVAL
    ApplicationWatchdog::checkin(); // resets the AWDT count
  #endif
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
      #ifdef WATCHDOG_INTERVAL
        ApplicationWatchdog::checkin(); // resets the AWDT count
      #endif
    }
} // setup_wifi()


String stats = "none yet";

void setup_cloud() {
  static bool already_set = false;

  if (! already_set 
      && Particle.connected()) {
    read_mode();
    Particle.variable("mode", mode_name);
    Particle.variable("statistics", stats);
    Particle.function("toggle_mode", toggle_mode);
    Particle.variable("uptime_h", uptime_h);
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


#ifdef WATCHDOG_INTERVAL
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
#endif


void delay_safely(int delay_ms) {
  int remainder = delay_ms;
  while (remainder > 1000) {
    delay(1000);
    remainder -= 1000;
    #ifdef WATCHDOG_INTERVAL
      ApplicationWatchdog::checkin(); // resets the AWDT count
    #endif
  }
  delay(remainder);
} // delay_safely(delay_ms)


void maybe_reconnect() {
  if (!Particle.connected()) {
      WiFi.off();
      delay_safely(30*1000);
      WiFi.on(); 
      delay_safely(30*1000);
      Particle.connect();
      delay_safely(10*1000); // Wait for connection attempt
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
    // synctime_or_die_trying();
                  
    #ifdef WATCHDOG_INTERVAL
      wd = new ApplicationWatchdog(WATCHDOG_INTERVAL * 1000, 
                                   watchdogHandler, 1536);
    #endif 

    setup_dst();
    setup_cloud();
    setup_nitetime();
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


TimedStatistics ma_sw = TimedStatistics(10);
TimedStatistics display_speed = TimedStatistics(10);
TimedStatistics engine = TimedStatistics(10);
Statistics nframes = Statistics(10);
Statistics avg_budget = Statistics(10);
SimpleTimer loop_pacer(MS_PER_FRAME);

void loop() {
    ma_sw.start();
    if (second.isExpired()) {
        maybe_reboot();  // only check this once per second
        uptime_h = 1.0 * millis() / (3600*1000);
        Log.info("free memory: %ld", System.freeMemory());
        chef.cook(food, wTime);
        luna_brite = luna->get_brightness();
        display_brite = display.set_brightness(luna_brite);
    }
    if (hourly.isExpired()) {
        maybe_reconnect();
    }
    
    display.clear();

    check_nitetime(wTime.hour(), wTime.minute());
    if (is_nitetime) {
      display.nite_render(food);
    } else {
      engine.start();
      loop_whatever_mode();
      engine.stop();

      maybe_update_layout();
    
      if (layout.show_weather) {
        Log.trace("checkpoint 1");
        delay(100);
        weatherGFX->run(weather.icon_str);
        Log.trace("checkpoint 2");
        delay(100);
        display.render(weatherGFX->peers, MATRIX_Y);
        Log.trace("checkpoint 3");
        delay(100);
        temperature_graph->update(weather.feels_like());
        Log.trace("checkpoint 4");
        delay(100);
        display.render(temperature_graph->dots, MATRIX_Y);
        Log.trace("checkpoint 5");
        delay(100);
      }
    
      if (layout.show_plan) {
        display.render(food);
      }

      if (layout.show_pinger) {
        display.render(pinger.pings(), pinger.npings());
      }
      display.render(sandbox);
      display_speed.start();
      // display.show_multipass();
      int budget_ms = max(0, MS_PER_FRAME - engine.read());
      avg_budget.add(budget_ms);
      nframes.add(display.show(budget_ms));
      // display.show();
      display_speed.stop();
    }
    ma_sw.stop();
    stats = String::format("MS per frame: %u; display speed: %u; engine: %u; frames per frame: %d, budget %d ms", 
                           ma_sw.read(), display_speed.read(), engine.read(),
                           nframes.read(), avg_budget.read());
    loop_pacer.wait();
} // loop()
