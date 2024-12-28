#pragma once

#include <SimpleTimer.h>

#define LOWER_BRIGHT 0 // lowest brightness to return
#define UPPER_BRIGHT 100 // highest brightness to return

#define PRINTF_DEBUGGER

/*
 * Luna: the thing which manages light / brightness
 *
 * Calibration:
 *   shine a light at the sensor
 *   check the luna_sensor value
 *   call SetBrightSensor with this value (or one near it)
 *   cover the sensor
 *   check the value
 *   call setDimSensor with this value
 *
 * Test: 
 *   shine a light at the sensor
 *     observe clock brightening
 *   cover the sensor
 *     observe clock darkening
 *
 */

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
        Luna(int, int, int, int);
        ~Luna() { delete hysteresis; }
        void setup_cloud();
        int get_brightness();
        int setDimSensor(String);
        int setBrightSensor(String);
}; // class Luna


float powMap(float value, float exponent, float inMin, float inMax, float outMin, float outMax) {
  // Map input value to [0, 1] using pow(exponent) scaling
  float normalized = (pow(value, exponent) - pow(inMin, exponent)) / (pow(inMax, exponent) - pow(inMin, exponent));

  // Map normalized value to output range
  return outMin + normalized * (outMax - outMin);
} // float powMap(value, exp, min, max, min, max)


Luna::Luna(int power_pin, int sense_pin, int ground_pin, int newAddress) {
    address = newAddress;
    pin = sense_pin;
    pinMode(power_pin, OUTPUT);
    digitalWrite(power_pin, HIGH);
    pinMode(ground_pin, OUTPUT);
    digitalWrite(ground_pin, LOW);
    pinMode(pin, INPUT);
    brightness = 0;
    hysteresis = new SimpleTimer(5*1000);
    last_read = 1;
    read_data();
    get_brightness();
} // Luna(power, sense, ground, addy)


void Luna::setup_cloud() {
    Particle.function("luna_dim_sensor", &Luna::setDimSensor, this);
    Particle.function("luna_bright_sensor", &Luna::setBrightSensor, this);
    
    Particle.variable("luna_sensor", this->sensor_value);
    Particle.variable("luna_brightness", this->brightness);
} // setup_cloud()


// takes a sensor value for "dimmest lighting"
int Luna::setDimSensor(String data) {
    int value = data.toInt();
    if (value != 0) {
        dim_sensor = value;
        write_data();
    }
    return dim_sensor;
} // setDimSensor(data)


// takes a sensor value for "brightest lighting"
int Luna::setBrightSensor(String data) {
    int value = data.toInt();
    if (value != 0) {
        bright_sensor = value;
        write_data();
    }
    return bright_sensor;
} // setBrightSensor(data)


void Luna::read_data() {
    EEPROM.get(address, dim_sensor);
    EEPROM.get(address+sizeof(dim_sensor), bright_sensor);
} // read_data()


void Luna::write_data() {
    EEPROM.put(address, dim_sensor);
    EEPROM.put(address+sizeof(dim_sensor), bright_sensor);
} // write_data()


// abs(sensor_value - last_read) > abs(bright_sensor - dim_sensor) / 25;
// returns [0..99]
int Luna::get_brightness() {
    sensor_value = analogRead(pin);
    Log.trace("luna: sensor_value=%d", sensor_value);
    int tmp = constrain(powMap(sensor_value, 2, dim_sensor, bright_sensor, LOWER_BRIGHT, UPPER_BRIGHT),
                               LOWER_BRIGHT, UPPER_BRIGHT);
    Log.trace("tmp=%d", tmp);
    if (sensor_value > 0 && hysteresis->isExpired() && abs(tmp - brightness) > 2) {
        brightness = tmp;
    }
    return brightness;
} // int get_brightness()
