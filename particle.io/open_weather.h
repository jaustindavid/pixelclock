#ifndef OPENWEATHER_H
#define OPENWEATHER_H

#include "Particle.h"
#include <SimpleTimer.h>
#include <HttpClient.h>
#include <ArduinoJson.h>



// call this frequently, it will occasionally hit the openweather API
class OpenWeather {
    private:
        SimpleTimer* timer;
        float _feels_like_temp;
        int _icon;
        const char* API_KEY = "abede4bd54d77c92a10495b460e9de5a";
        const char* ZIP_CODE = "29414";


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
            
            request.path = String::format("/data/2.5/weather?q=%s&appid=%s&units=metric", ZIP_CODE, API_KEY);
            request.hostname = "api.openweathermap.org";
            request.port = 80;
        
            // Get request
            Serial.printf("calling http://%s%s\n", request.hostname.c_str(), request.path.c_str());
            unsigned long start = millis();
            http.get(request, response, headers);
            Serial.print("Application>\tResponse status: ");
            Serial.println(response.status);
            unsigned long elapsed = millis() - start;
            Serial.printf("%d ms\n", elapsed);
        
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
                return;
            }
        
            // Fetch the values
            _feels_like_temp = doc["main"]["feels_like"];
            const char * icon = doc["weather"][0]["icon"];
            String s = String(icon);
            _icon = s.toInt();
            Serial.printf("feels like %5.2f, icon: %s=>%d\n", _feels_like_temp, icon, _icon);
        }
        
        
    public:
        OpenWeather(int update_period) : _feels_like_temp(-99.0) {
            timer = new SimpleTimer(update_period, true);
            // Particle.variable("feels_like", _feels_like_temp);
            update();
        }


        ~OpenWeather() {
            delete timer;
        }


        float feels_like() {
            if (timer->isExpired()) {
                update();
            }
            return _feels_like_temp;
        }
        
        float icon() {
            if (timer->isExpired()) {
                update();
            }
            return _icon;
        }
};






void getWeatherData() {
}
#endif