#ifndef PINGER_H
#define PINGER_H

#include <SimpleTimer.h>
#include "dot.h"
#include "ant.h"

// holds a graph, which is actually a set of dots
// returns it on request
class Pinger {
    private:
        Dot* graph[MATRIX_X];
        SimpleTimer* ping_timer;

        // each ping() call does one timed ping, returns latency (ms) or -1
        int ping() {
            // static byte strength = 0;
            Serial.println("Pinging out...");
            static IPAddress innernet(8,8,8,8);
            unsigned long start = millis();
            byte n = WiFi.ping(innernet, 1);
            
            if (n == 0) {
                Serial.println("failed");
                return -1;
            } else {
                int duration = (millis() - start);
                Serial.printf("Success; duration = %d ms\n", duration);
                return duration;
            }
        } // ping()


        void update_graph() {
            // propagate colors to the left
            for (int i = 0; i < MATRIX_X-1; i++) {
               graph[i]->color = graph[i+1]->color;
            }
            int latency = ping();
            int r = map(latency, 50, 500, 0, 255);
            int g = map(latency, 0, 250, 255, 0);
            // Serial.printf("latecy %d -> r = %d, g = %d\n", latency, r, g);
            // Dot* dot =graph[MATRIX_X - 1];
            if (latency == -1 || latency > 500) {
                // Serial.println("reddenning");
                graph[MATRIX_X-1]->set_color(RED);
            } else if (latency < 50) {
                // Serial.println("greenenning");
                graph[MATRIX_X-1]->set_color(GREEN);
            } else { 
                // Serial.printf("coloring r = %d, g = %d\n", latency, r, g);
                graph[MATRIX_X-1]->set_color(Adafruit_NeoPixel::Color(r, g, 0));
            }
           //  Serial.printf("Finally: color = %08x @ (%d,%d)\n", graph[MATRIX_X-1]->color, graph[MATRIX_X-1]->x, graph[MATRIX_X-1]->y);
        }
        
        
    public:
        Pinger() {
            ping_timer = new SimpleTimer(15*1000);
            for (int i = 0; i < MATRIX_X; i++) {
                graph[i] = new Ant();
                graph[i]->x = i;
                graph[i]->y = MATRIX_Y - 1;
                graph[i]->color = (Adafruit_NeoPixel::Color(0, 0, (i+1)*16-1));
                graph[i]->active = true;
            }
        }
        
        ~Pinger() {
            delete ping_timer;
        }
        
        
        // return a graph of ping data
        Dot** pings() {
            // Serial.println("getting pings");
            if (ping_timer->isExpired()) {
                Serial.println("pinger: updating graph");
                update_graph();
            }
            
            return graph;
        }
};

#endif