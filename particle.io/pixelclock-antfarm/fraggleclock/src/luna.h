#ifndef LUNA_H
#define LUNA_H

#include <SimpleTimer.h>

#define LOWER_BRIGHT 0 // lowest brightness to return
#define UPPER_BRIGHT 100 // highest brightness to return

#define PRINTF_DEBUGGER


class Luna {
    private:
        int pin;
        int last_read;
        int brightness, dim_sensor, bright_sensor;
        SimpleTimer *hysteresis;
        int address;
        void read_data();
        void write_data();
        
    public:
        int sensor_value;
        Luna(int, int);
        ~Luna() { delete hysteresis; }
        void setup();
        void init();
        int get_brightness();
        int setDimSensor(String);
        int setBrightSensor(String);
};


float powMap(float value, float exponent, float inMin, float inMax, float outMin, float outMax) {
  // Map input value to [0, 1] using pow(exponent) scaling
  float normalized = (pow(value, exponent) - pow(inMin, exponent)) / (pow(inMax, exponent) - pow(inMin, exponent));

  // Map normalized value to output range
  return outMin + normalized * (outMax - outMin);
}


Luna::Luna(int new_pin, int newAddress) {
    address = newAddress;
    pin = new_pin;
    pinMode(A0, OUTPUT);
    digitalWrite(A0, HIGH);
    pinMode(A2, OUTPUT);
    digitalWrite(A2, LOW);
    pinMode(pin, INPUT);
    hysteresis = new SimpleTimer(5*1000);
    last_read = 1;
    read_data();
    get_brightness();
}


void Luna::setup() {
    /*
    Particle.function("luna_dim_sensor", &Luna::setDimSensor, this);
    Particle.function("luna_bright_sensor", &Luna::setBrightSensor, this);
    
    Particle.variable("luna_sensor", this->sensor_value);
    Particle.variable("luna_brightness", this->brightness);
    */
}


int Luna::setDimSensor(String data) {
    int value = data.toInt();
    if (value != 0) {
        dim_sensor = value;
        write_data();
    }
    return dim_sensor;
}


int Luna::setBrightSensor(String data) {
    int value = data.toInt();
    if (value != 0) {
        bright_sensor = value;
        write_data();
    }
    return bright_sensor;
}


void Luna::read_data() {
    EEPROM.get(address, dim_sensor);
    EEPROM.get(address+sizeof(dim_sensor), bright_sensor);
}


void Luna::write_data() {
    EEPROM.put(address, dim_sensor);
    EEPROM.put(address+sizeof(dim_sensor), bright_sensor);
}


// abs(sensor_value - last_read) > abs(bright_sensor - dim_sensor) / 25;
// returns [0..99]
int Luna::get_brightness() {
    sensor_value = analogRead(pin);
    int tmp = constrain(powMap(sensor_value, 2, dim_sensor, bright_sensor, LOWER_BRIGHT, UPPER_BRIGHT),
                               LOWER_BRIGHT, UPPER_BRIGHT);
    Log.trace("luna: sensor_value=%d, tmp=%d", sensor_value, tmp);
    if (sensor_value > 0 && hysteresis->isExpired() && abs(tmp - brightness) > 2) {
        brightness = tmp;
    }
    return brightness;
}

#endif
