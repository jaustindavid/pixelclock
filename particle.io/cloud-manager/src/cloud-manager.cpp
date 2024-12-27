/* 
 * Project myProject
 * Author: Your Name
 * Date: 
 * For comprehensive documentation and examples, please visit:
 * https://docs.particle.io/firmware/best-practices/firmware-template/
 */

// Include Particle Device OS APIs
#include "Particle.h"
#include <SimpleTimer.h>
#include <neopixel.h>

#include "safer-cloud.h"

// SerialLogHandler logHandler(LOG_LEVEL_INFO);
SerialLogHandler logHandler(LOG_LEVEL_TRACE);


#define PHOTON2 32

#ifndef PIXEL_PIN
  #if (PLATFORM_ID == PHOTON2)
    #define PIXEL_PIN SPI
  #else //
    #define PIXEL_PIN D0
  #endif
#endif

#define PIXEL_COUNT 256
#define PIXEL_TYPE WS2812B
Adafruit_NeoPixel neopixels(PIXEL_COUNT, PIXEL_PIN, PIXEL_TYPE);


#define RED    0xFF0000
#define GREEN  0x00FF00
#define BLUE   0x0000FF
#define YELLOW 0xFFFF00

void show_connection_status(int i = 0) {
  Log.trace("show_connection_status(): starting");
  if (Particle.connected()) {
    neopixels.setPixelColor(i%PIXEL_COUNT, GREEN);
    Log.trace("show_connection_status(): connected!");
  } else if (WiFi.ready()) {
    neopixels.setPixelColor(i%PIXEL_COUNT, YELLOW);
    Log.trace("show_connection_status(): not connected, but WiFi ready!");
  } else {
    Log.trace("show_connection_status(): not connected, no WiFi");
    neopixels.setPixelColor(i%PIXEL_COUNT, RED);
  }

  neopixels.show();
  Log.trace("show_connection_status(): done");
}


void setup() {
  Serial.begin(115200);
  waitFor(Serial.isConnected, 10000);
  delay(1000);

  neopixels.begin();
  neopixels.setBrightness(16);
  neopixels.clear();
  show_connection_status();
  neopixels.show();
  synctime_or_die_trying();
}


void loop() {
  neopixels.clear();
  neopixels.show();
  Log.info("looping; free mem = %u", System.freeMemory());
  Log.info("Time: %02d:%02d", Time.hour(), Time.minute());
  show_connection_status();
  safe_connect();
  safe_synctime();
  if (safe_publish("test event", "boop")) {
    Log.info("published");
  } else {
    Log.info("not published");
  }
  // safe_disconnect(30000);
  for (int i = 0; i < 24; i++) {
    show_connection_status(i);
    delay(5000);
  }
}
