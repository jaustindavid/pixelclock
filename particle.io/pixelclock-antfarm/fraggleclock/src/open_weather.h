#ifndef OPENWEATHER_H
#define OPENWEATHER_H

#include "Particle.h"
#include <SimpleTimer.h>
#include <HttpClient.h>
#include <ArduinoJson.h>

#include "defs.h"
#include "ow_api.h"



// call this frequently, it will occasionally hit the openweather API
class OpenWeather {
    private:
        int address;
        SimpleTimer* timer;
        float _feels_like_temp;
        int _icon, forecast_age;
        time_t last_update;
        const char* API_KEY = OW_API_KEY;
        double lattitude, longitude;


        void update() {
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
            icon_str = String((const char *)doc["weather"][0]["icon"]);
            _icon = icon_str.toInt();
            Serial.printf("feels like %5.2f, icon: %s=>%d\n", _feels_like_temp, icon_str.c_str(), _icon);
            Particle.publish("openweather", 
                String::format("feels like %5.2f, icon: %s=>%d\n", 
                        _feels_like_temp, icon_str.c_str(), _icon));
            last_update = millis();
        }
        
        
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
            return;
            EEPROM.get(address, lattitude);
            EEPROM.get(address+sizeof(lattitude), longitude);
        } // read_data()


        void write_data() {
            struct weather_datum datum = 
            { .version = 1,
              .lat = lattitude,
              .lon = longitude,
            };
            EEPROM.put(address, datum);
            return;
            EEPROM.get(address, datum);
            double d;
            EEPROM.update(address, d);
            EEPROM.get(address, d);
            if (lattitude != d) {
                EEPROM.put(address, lattitude);
            }
            EEPROM.get(address+sizeof(lattitude), d);
            if (longitude != d) {
                EEPROM.put(address+sizeof(lattitude), longitude);
            }
        }


        int setLattitude(String data) {
            float f = data.toFloat();
            if (f != 0) {
                lattitude = (double)f;
                write_data();
            }
            last_update = 0; // inspire an update
            return (int)(lattitude * 100);
        } // int setLattitude(String data)
        

        int setLongitude(String data) {
            float f = data.toFloat();
            if (f != 0) {
                longitude = (double)f;
                write_data();
            }
            last_update = 0; // inspire an update
            return (int)(longitude * 100);
        } // int setLongitude(String data)


    public:
        String icon_str;

        OpenWeather(int addy, int update_period) : _feels_like_temp(-99.0) {
            address = addy;
            timer = new SimpleTimer(update_period, true);
            // Particle.variable("feels_like", _feels_like_temp);
            lattitude = 32.85;
            longitude = -80.06;
            icon_str = "01d";
            update();
        }


        ~OpenWeather() {
            delete timer;
        }


        void setup() {
            Particle.function("ow_set_lattitude", &OpenWeather::setLattitude, this);
            Particle.function("ow_set_longitude", &OpenWeather::setLongitude, this);
            Particle.variable("ow_lattitude", this->lattitude);
            Particle.variable("ow_longitude", this->longitude);
            Particle.variable("ow_age", this->forecast_age);
            read_data();
        }
        

        float feels_like() {
            if (timer->isExpired()) {
                update();
            }
            forecast_age = (millis() - last_update)/1000;
            return _feels_like_temp;
        }

        
        float icon() {
            if (timer->isExpired()) {
                update();
            }
            forecast_age = (millis() - last_update)/1000;
            return _icon;
        }
};


#endif
