#pragma once

#include "Particle.h"
#include <SimpleTimer.h>
#include <HttpClient.h>
#include <ArduinoJson.h>

#include "defs.h"
#include "ow_api.h"

#define OW_DEBUG

#define MAX_FORECAST_AGE 3600 // allowed to be an hour old


// call this frequently, it will occasionally hit the openweather API
class OpenWeather {
    private:
        int address;
        SimpleTimer* update_timer;
        bool update_me;
        float _feels_like_temp;
        String _icon;
        int forecast_age;
        time_t last_update;
        const char* API_KEY = OW_API_KEY;
        double lattitude, longitude;


        void update() {
            if (! WiFi.ready()) {
                Log.warn("OW: WiFi not ready in update()");
                update_timer->reset();
                return;
            }
            // Headers currently need to be set at init, useful for API keys etc.
            http_header_t headers[] = {
                //  { "Content-Type", "application/json" },
                //  { "Accept" , "application/json" },
                { "Accept" , "*/*"},
                { NULL, NULL } // NOTE: Always terminate headers will NULL
            };
            
            HttpClient http;
            http_request_t request;
            http_response_t response;
            
            request.path = String::format("/data/2.5/weather?lat=%5.2f&lon=%5.2f&appid=%s&units=metric", lattitude, longitude, API_KEY);
            request.hostname = "api.openweathermap.org";
            request.port = 80;
        
            // Get request
            Serial.printf("calling http://%s%s\n", request.hostname.c_str(), request.path.c_str());
            unsigned long start = millis();
            http.get(request, response, headers);
            Serial.print("Application>\tResponse status: ");
            Serial.println(response.status);
            Particle.publish("openweather",
                String::format("%d: http://%s%s", 
                        response.status,
                        request.hostname.c_str(), 
                        request.path.c_str()));
            unsigned long elapsed = millis() - start;
            Serial.printf("%ld ms\n", elapsed);
        
            Serial.print("Application>\tHTTP Response Body: ");
            Serial.println(response.body);
            // http.responseBody();
            
            // Allocate the JSON document
            JsonDocument doc;
        
            // Deserialize the JSON document
            DeserializationError error = deserializeJson(doc, response.body.c_str());
        
            // Test if parsing succeeds
            if (error) {
                Serial.print(F("deserializeJson() failed: "));
                Serial.println(error.c_str());
                Particle.publish("openweather", 
                    String::format("Error: %s", error.c_str()));
                return;
            }
        
            // Fetch the values
            _feels_like_temp = doc["main"]["feels_like"];
            _icon = String((const char *)doc["weather"][0]["icon"]);
            Log.warn("OW: feels like %5.2f, icon: %s", 
                     _feels_like_temp, _icon.c_str());
            Particle.publish("openweather", 
                String::format("feels like %.2f, icon: %s", 
                        _feels_like_temp, _icon.c_str()));
            last_update = millis();
        } // update()


        // update, if needed
        void maybe_update() {
            if (update_me 
                || update_timer->isExpired()) {
                update();
                update_me = false;
            }
            forecast_age = (millis() - last_update)/1000;
        } // maybe_update()
        
        
        struct weather_datum {
          byte version;
          double lat;
          double lon;
        };


        void read_data() {
            struct weather_datum datum;
            EEPROM.get(address, datum);
            if (datum.version == 1) {
              lattitude = datum.lat;
              longitude = datum.lon;
            };
        } // read_data()


        void write_data() {
            struct weather_datum datum = 
            { .version = 1,
              .lat = lattitude,
              .lon = longitude,
            };
            EEPROM.put(address, datum);
            return;
        } // write_data()


        int setLattitude(String data) {
            float f = data.toFloat();
            if (f != 0) {
                lattitude = (double)f;
                write_data();
            }
            update_me = true;
            return (int)(lattitude * 100);
        } // int setLattitude(String data)
        

        int setLongitude(String data) {
            float f = data.toFloat();
            if (f != 0) {
                longitude = (double)f;
                write_data();
            }
            update_me = true;
            return (int)(longitude * 100);
        } // int setLongitude(String data)


        #ifdef OW_DEBUG
        int set_feels_like(String data) {
          float t = data.toFloat();
          if (t) {
            _feels_like_temp = t;
            update_me = true;
          }
          return (int)_feels_like_temp;
        } // int set_feels_like(data)


        int set_icon(String data) {
          _icon = data;
          update_me = true;
          return _icon.toInt();
        } // int set_feels_like(data)
        #endif

    public:
        String icon_str;

        OpenWeather(int addy, int update_period) : _feels_like_temp(-99.0) {
            address = addy;
            update_timer = new SimpleTimer(update_period, true);
            lattitude = 32.85;
            longitude = -80.06;
            icon_str = "01d";
            _feels_like_temp = -99.0;
            read_data();
            last_update = 0;
            update_me = false;
            // TODO: remove this from init
            update();
        } // OpenWeather(addy, update_period)


        ~OpenWeather() {
            delete update_timer;
        } // ~OpenWeather()


        void setup_cloud() {
            Particle.function("ow_set_lattitude",
                              &OpenWeather::setLattitude, this);
            Particle.function("ow_set_longitude",
                              &OpenWeather::setLongitude, this);
            Particle.variable("ow_age", this->forecast_age);
            #ifdef TESTING
            #ifdef OW_DEBUG
              Particle.function("ow_feels_like", 
                                &OpenWeather::set_feels_like, this);
              Particle.function("ow_icon", 
                                &OpenWeather::set_icon, this);
            #endif
            #endif
        } // setup()
        

        // true if data is real and recent
        bool isValid() {
          maybe_update();
          return last_update 
                 && (forecast_age < MAX_FORECAST_AGE);
        } // bool isValid()


        float feels_like() {
            maybe_update();
            return _feels_like_temp;
        } // float feels_like()

        
        String icon() {
            maybe_update();
            return _icon;
        } // String icon()
};
