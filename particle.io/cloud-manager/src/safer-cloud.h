#pragma once

SYSTEM_MODE(SEMI_AUTOMATIC);
SYSTEM_THREAD(ENABLED);

#define LOGGING Log.info

// safely connect to the cloud
void safe_connect(int timeout = 0) {
  if (Particle.connected()) {
    LOGGING("safe_connect(): already connected, nothing to do");
    return;
  }

  LOGGING("safe_connect(): attempting connection...");
  Particle.connect();
  if (timeout) {
    waitFor(Particle.connected, timeout);
    LOGGING("safe_connect(): waitFor() has returned");
  }
} // safe_connect()


// safely sync with the cloud, not more than once per resync_period
void safe_synctime(uint32_t resync_period = 24*3600*1000) {
  static uint32_t lastSync = 0;

  LOGGING("safe_synctime(): starting...");
  if (lastSync 
      && Time.isValid()
      && millis() - lastSync < resync_period) {
    LOGGING("safe_synctime(): not required, skipping");
    return;
  }

  // Request time synchronization from the Particle Device Cloud
  if (Particle.connected()) {
    Particle.syncTime();
    waitFor(Time.isValid, 60000);
    if (Time.isValid()) {
      LOGGING("safe_synctime(): Time isValid; recording...");
      lastSync = millis();
    } else {
      LOGGING("safe_synctime(): Time is NOT Valid; bailing...");
    }
  }
} // safe_synctime(resync_period)


// exactly what it sounds like
void synctime_or_die_trying() {
  while (! Time.isValid()) {
    safe_connect(30000);
    safe_synctime();
  }
} // synctime_or_die_trying()


void safe_disconnect(int timeout = 0) {
  if (Particle.disconnected() && ! WiFi.ready()) {
    LOGGING("safe_disconnect(): already disconnected, nothing to do");
    return;
  }

  LOGGING("safe_disconnect(): breaking connection...");

  Particle.disconnect();

  waitFor(Particle.disconnected, min(timeout, 5000));
  LOGGING("safe_disconnect(): waitFor() has returned");
  // LOGGING("safe_disconnect(): turning off WiFi...");
  // WiFi.off();
  LOGGING("safe_disconnect(): done.");
} // safe_disconnect()


bool safe_publish(const char* event, const char* data) {
  if (Particle.connected()) {
    LOGGING("safe_publish(): connected, attempting publish()");
    return Particle.publish(event, data);
  }
  LOGGING("safe_publish(): not connected, skipping publish()");
  return false;
} // safe_publish(event, data)
