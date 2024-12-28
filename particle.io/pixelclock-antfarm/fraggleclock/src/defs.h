#pragma once

#define PHOTON2 32

#define HOLDING_PATTERN 60 // seconds before firing up
// to enable the watchdog...
#define WATCHDOG_INTERVAL 180000 // milliseconds

#define SQUARE     0
#define WIDESCREEN 1

#if (ASPECT_RATIO == WIDESCREEN)
  #define MATRIX_X 32
  #define MATRIX_Y 8
  #define MAX_IQ 34
#else
  #define MATRIX_X 16
  #define MATRIX_Y 16
  #define MAX_IQ 25
#endif
#define PIXEL_COUNT 256


#define MS_PER_FRAME      100 // a budget
#define REDRAW_SPEED_MS   25 // ms
#define REDRAWS_PER_FRAME 2  // implies 100ms peak frame rate, 10 FPS


#define PRINTF_DEBUGGER
#define MAX_DOTS 75

#define CORE_ADDY       0     // byte, bool, bool == 3 bytes
#define LUNA_ADDY       10    // int, int == 8 bytes
#define DISPLAY_ADDY    20    // 4 bytes
#define WT_ADDY         30    // int, int == 8 bytes
#define WEATHER_ADDY    40    // byte, double, double = 17 bytes
#define PINGER_ADDY     58    // bool == 1 byte
#define WIFI_ADDY       100   // 100 bytes
#define COLOR_ADDY      200   // 17 bytes
#define LAYOUT_ADDY     220   // 4 bytes

#define WIFI_EMERGENCY_SSID "raccoontime"
#define WIFI_EMERGENCY_PASSWD "busyness"

// massively globals
bool show_weather = false;
bool show_pinger = false;
bool show_food = false;

// (x, y) -> [i]
int txlate(int x, int y) {
    int pixel = 0;

    pixel = x * MATRIX_Y;
    if (x%2 == 0) {
        pixel += y % MATRIX_Y;
    } else {
        pixel += (MATRIX_Y - 1) - (y % MATRIX_Y);
    }

    return pixel;
} // int txlate(x, y)


void storeString(int start_address, String data) {
    char c;
    int i = 0;
    do {
        c = data.charAt(i);
        EEPROM.write(start_address+i, c);
        i++;
    } while (i < 50 && c);
    EEPROM.put(start_address+i, 0);
} // storeString(start_address, data)


String fetchString(int start_address) {
    char c, buffer[50];
    int i = 0;
    do {
        c = EEPROM.read(start_address+i);
        buffer[i] = c;
        i++;
    } while (i < 50 && c);
    // buffer[i] = 0;
    return String(buffer);
} // String fetchString(start_address)


class Stopwatch {
  private:
    uint32_t counter;
    uint32_t start_ms;

  public:
    Stopwatch() {
      reset();
    } // Stopwatch()

    void reset() {
      start_ms = 0;
      counter = 0;
    } // reset()

    void start() {
      start_ms = millis();
    }  // start()

    void stop() {
      if (start_ms) {
        counter += (millis() - start_ms);
        start_ms = 0;
      }
    } // stop()

    uint32_t read() {
      return counter;
    }
};


// Usage: 
//   start();
//   stop();
//   read() -> gives the average time of previous N
class MovingAverageStopwatch {
  private:
    Stopwatch stopwatch;
    int ma_target;
    int n_events;
    uint32_t sum;
    uint32_t avg;


  public:
    MovingAverageStopwatch(int n) {
      ma_target = n;
      n_events = 0;
      sum = 0;
      avg = 0;
    }


    void start() {
      stopwatch.start();
    }


    void stop() {
      stopwatch.stop();
      sum += stopwatch.read();
      stopwatch.reset();
      n_events ++;
      if (n_events > ma_target) {
        sum -= avg;
        n_events = ma_target;
      }
      avg = sum/n_events;
    }


    uint32_t read() {
      return avg;
    }
};
