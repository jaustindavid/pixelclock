#ifndef PINGER_H
#define PINGER_H

#include <SimpleTimer.h>
#include "dot.h"
#include "ant.h"

// holds a graph, which is actually a set of dots
// returns it on request
class Pinger {
    private:
        #define GRAPH_MIN 1
        #define GRAPH_MAX (MATRIX_X-2)
        Dot* graph[MATRIX_X];
        SimpleTimer* ping_timer;

        // each ping() call does one timed ping, returns latency (ms) or -1
        int ping() {
            // static byte strength = 0;
            Serial.println("Pinging out...");
            static IPAddress innernet(8,8,8,8);
            unsigned long start = millis();
            if (!Particle.connected()) {
                Particle.connect();
            }
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
            for (int i = GRAPH_MIN; i < GRAPH_MAX; i++) {
               graph[i]->color = graph[i+1]->color;
            }
            int latency = ping();
            int r = map(latency, 50, 500, 0, 255);
            int g = map(latency, 0, 250, 255, 0);

            // Serial.printf("updating i=%d, x=%d, %s\n", GRAPH_MAX, graph[GRAPH_MAX]->x, graph[GRAPH_MAX]->active ? "on" : "off");
            if (latency == -1 || latency > 500) {
                // Serial.println("reddenning");
                graph[GRAPH_MAX]->set_color(RED);
            } else if (latency < 50) {
                // Serial.println("greenenning");
                graph[GRAPH_MAX]->set_color(GREEN);
            } else { 
                // Serial.printf("coloring r = %d, g = %d\n", latency, r, g);
                graph[GRAPH_MAX]->set_color(Adafruit_NeoPixel::Color(r, g, 0));
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
                if (i >= GRAPH_MIN && i <= GRAPH_MAX) {
                    graph[i]->active = true;
                } else {
                    graph[i]->active = false;
                }
            }
        }


        ~Pinger() {
            delete ping_timer;
        }
        
        
        // return a graph of ping data
        Dot** pings() {
            // Serial.println("getting pings");
            if (ping_timer->isExpired()) {
                // Serial.println("pinger: updating graph");
                // Serial.printf("graph: [%d,%d]\n", GRAPH_MIN, GRAPH_MAX);
                update_graph();
            }
            
            return &graph[GRAPH_MIN];
        }


        int npings() {
            return GRAPH_MAX - GRAPH_MIN + 1;
        }
};

#endif
