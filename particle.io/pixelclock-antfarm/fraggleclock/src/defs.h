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
#define WIFI_ADDY       100

#define MIN_BRIGHTNESS 4     // the LOWEST brightness to display
#define MAX_BRIGHTNESS 64    // the HIGHEST brightness to display


void storeString(int start_address, String data) {
    char c;
    int i = 0;
    do {
        c = data.charAt(i);
        EEPROM.write(start_address+i, c);
        i++;
    } while (i < 50 && c);
    EEPROM.put(start_address+i, 0);
}


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
}


#endif
