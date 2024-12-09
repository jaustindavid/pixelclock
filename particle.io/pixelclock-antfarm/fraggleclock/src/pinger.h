#ifndef PINGER_H
#define PINGER_H

#include <SimpleTimer.h>
#include "defs.h"
#include "dot.h"
#include "ant.h"


#define PINGER_Y (MATRIX_Y-1)
#if (ASPECT_RATIO == SQUARE)
  #define PINGER_X 1       // first spot
  #define PINGER_WIDTH 14  // total width
#else
  #define PINGER_X 3       // first spot
  #define PINGER_WIDTH 26  // total width
#endif

// holds a graph, which is actually a set of dots
// returns it on request
class Pinger {
    private:
        Dot* graph[PINGER_WIDTH];
        SimpleTimer* ping_timer;


        // each ping() call does one timed ping, returns latency (ms) or -1
        int ping() {
            // static byte strength = 0;
            Log.warn("Pinging out...");
            static IPAddress innernet(8,8,8,8);
            unsigned long start = millis();
            if (!Particle.connected()) {
                Particle.connect();
            }
            byte n = WiFi.ping(innernet, 1);
            
            if (n == 0) {
                Log.warn("Failed");
                return -1;
            } else {
                int duration = (millis() - start);
                Log.warn("Success; duration = %d ms\n", duration);
                return duration;
            }
        } // ping()


        void update_graph() {
            // propagate colors to the left
            for (int i = 0; i < PINGER_WIDTH-1; i++) {
               graph[i]->color = graph[i+1]->color;
            }
            int latency = ping();
            int r = map(latency, 50, 500, 0, 255);
            int g = map(latency, 0, 250, 255, 0);

            /*
            Log.trace("updating i=%d, x=%d, %s\n", 
                 PINGER_WIDTH-1,
                 graph[PINGER_WIDTH-1]->x, 
                 graph[PINGER_WIDTH-1]->active ? "on" : "off");
            */
            if (latency == -1 || latency > 500) {
                // Serial.println("reddenning");
                graph[PINGER_WIDTH-1]->set_color(RED);
            } else if (latency < 50) {
                // Serial.println("greenenning");
                graph[PINGER_WIDTH-1]->set_color(GREEN);
            } else { 
                graph[PINGER_WIDTH-1]->set_color(
                    Adafruit_NeoPixel::Color(r, g, 0));
            }
           //  Serial.printf("Finally: color = %08x @ (%d,%d)\n", graph[MATRIX_X-1]->color, graph[MATRIX_X-1]->x, graph[MATRIX_X-1]->y);
        } // update_graph()
        
        
    public:
        Pinger() {
            ping_timer = new SimpleTimer(15*1000);
        } // Pinger()


        void setup() {
            for (int i = 0; i < PINGER_WIDTH; i++) {
                graph[i] = new Dot();
                graph[i]->x = PINGER_X + i;
                graph[i]->y = PINGER_Y;
                graph[i]->color = (Adafruit_NeoPixel::Color(0, 0, (i+1)*16-1));
                graph[i]->active = true;
            }
        } // setup(width)


        ~Pinger() {
            delete ping_timer;
        } // ~Pinger()
        

        void loop() {
            if (ping_timer->isExpired()) {
                // Serial.println("pinger: updating graph");
                // Serial.printf("graph: [%d,%d]\n", GRAPH_MIN, GRAPH_MAX);
                update_graph();
            }
        } // loop()

        
        // return a graph of ping data
        Dot** pings() {
            // Log.trace("getting pings");
            if (ping_timer->isExpired()) {
                // Serial.println("pinger: updating graph");
                // Serial.printf("graph: [%d,%d]\n", GRAPH_MIN, GRAPH_MAX);
                update_graph();
            }
            
            return &graph[0];
        } // Dot** pings()


        int npings() {
            // return GRAPH_MAX - GRAPH_MIN + 1;
            return PINGER_WIDTH;
        } // npings()
};

#endif
