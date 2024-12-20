#ifndef DEFS_H
#define DEFS_H

#define PHOTON2 32

#include "aspect.h"

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


#define REDRAW_SPEED_MS 50 // ms
#define REDRAWS_PER_FRAME 5 // implies 500ms frame rate

#define HOLDING_PATTERN 10 // seconds before goin nuts

#define PRINTF_DEBUGGER
#define MAX_DOTS 75

#define CORE_ADDY       0     // byte, bool, bool == 3 bytes
#define LUNA_ADDY       10    // int, int == 8 bytes
#define DISPLAY_ADDY    20    // 4 bytes
#define WT_ADDY         30    // int, int == 8 bytes
#define WEATHER_ADDY    40    // byte, double, double = 17 bytes
#define PINGER_ADDY     58    // bool == 1 byte
#define WIFI_ADDY       100   // 100 bytes
#define COLOR_ADDY      200   // 4 bytes

#define WIFI_EMERGENCY_SSID "raccoontime"
#define WIFI_EMERGENCY_PASSWD "busyness"

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


#endif
