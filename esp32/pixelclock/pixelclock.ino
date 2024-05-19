#include "soft_ap.h"

SoftAP *softAP;

void setup() {
  Serial.begin(115200);
  Serial.println("beginning");

  softAP = new SoftAP();
  softAP->setup();
  softAP->loop_forever();
}

void loop() {
}