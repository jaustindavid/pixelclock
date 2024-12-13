#ifndef DEFS_H
#define DEFS_H

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

#define PRINTF_DEBUGGER
#define MAX_DOTS 75

#define CORE_ADDY       0
#define LUNA_ADDY       10
#define DISPLAY_ADDY    20
#define WT_ADDY         30
#define WEATHER_ADDY    40
#define PINGER_ADDY     50
#define WIFI_ADDY       100

#define MIN_BRIGHTNESS 4     // the LOWEST brightness to display
#define MAX_BRIGHTNESS 128    // the HIGHEST brightness to display

#define TXLATE(X, Y) (X + Y*MATRIX_Y)


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
