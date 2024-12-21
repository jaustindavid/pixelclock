#ifndef PINGER_H
#define PINGER_H

#include <SimpleTimer.h>
#include "defs.h"
#include "dot.h"
#include "ant.h"


#define PINGER_X 2            // first spot
#define PINGER_Y (MATRIX_Y-1) // bottom row

#if (ASPECT_RATIO == SQUARE)
  #define DEFAULT_PINGER_WIDTH 12
#else
  #define DEFAULT_PINGER_WIDTH 28
#endif

#define MAX_RGB 96
#define REDDISH  (Adafruit_NeoPixel::Color(MAX_RGB, 0, 0))
#define GREENISH (Adafruit_NeoPixel::Color(0, MAX_RGB, 0))

// holds a graph, which is actually a set of dots
// returns it on request
class Pinger {
    private:
        Dot* graph[MATRIX_Y];
        SimpleTimer* ping_timer;
        bool show_pinger;
        int x, width;


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
            for (int i = 0; i < MATRIX_Y - 1; i++) {
               graph[i]->color = graph[i+1]->color;
            }

            int latency = ping();
            int r = map(latency, 50, 500, 0, MAX_RGB);
            int g = map(latency, 0, 250, MAX_RGB, 0);

            /*
            Log.trace("updating i=%d, x=%d, %s\n", 
                 PINGER_WIDTH-1,
                 graph[PINGER_WIDTH-1]->x, 
                 graph[PINGER_WIDTH-1]->active ? "on" : "off");
            */
            int index = x + width - 1;
            if (latency == -1 || latency > 500) {
                // Serial.println("reddenning");
                graph[index]->set_color(REDDISH);
            } else if (latency < 50) {
                // Serial.println("greenenning");
                graph[index]->set_color(GREENISH);
            } else { 
                graph[index]->set_color(Adafruit_NeoPixel::Color(r, g, 0));
            }
           //  Serial.printf("Finally: color = %08x @ (%d,%d)\n", graph[MATRIX_X-1]->color, graph[MATRIX_X-1]->x, graph[MATRIX_X-1]->y);
        } // update_graph()
        
        
        void setup_cloud_functions() {
          Particle.function("toggle_pinger", &Pinger::toggle_pinger, this);
        } // setup_cloud_functions()
        

        int toggle_pinger(String data) {
          // show_pinger = (show_pinger ? false : true);
          show_pinger = !show_pinger;
          write_eeprom();
          return (int)show_pinger;
          if (show_pinger) {
            return 42;
          } else {
            return 69;
          }
        } // toggle_pinger(data)


        void read_eeprom() {
          show_pinger = EEPROM.read(PINGER_ADDY);
          // EEPROM.get(PINGER_ADDY, show_pinger);
        } // read_eeprom()


        void write_eeprom() {
          EEPROM.put(PINGER_ADDY, show_pinger);
          if (EEPROM.read(PINGER_ADDY) != show_pinger) {
            EEPROM.write(PINGER_ADDY, show_pinger);
          }
        } // write_eeprom()


    public:
        Pinger() {
            ping_timer = new SimpleTimer(15*1000);
            show_pinger = false;
            width = DEFAULT_PINGER_WIDTH;
            x = PINGER_X;
        } // Pinger()


        void setup() {
            for (int i = 0; i < MATRIX_Y; i++) {
                graph[i] = new Dot();
                graph[i]->x = PINGER_X + i;
                graph[i]->y = PINGER_Y;
                graph[i]->color = (Adafruit_NeoPixel::Color(0, 0, (i+1)*16-1));
                graph[i]->active = true;
            }

            setup_cloud_functions();
            read_eeprom();

            set_layout(PINGER_X, DEFAULT_PINGER_WIDTH);
        } // setup(width)


        ~Pinger() {
            delete ping_timer;
        } // ~Pinger()


        void set_layout(int start_x, int new_width) {
          x = start_x;
          width = new_width;
          for (int i = 0; i < MATRIX_Y; i++) {
            if (graph[i]->x < start_x
                || graph[i]->x >= (start_x + width)) {
              graph[i]->active = false;
            } else {
              graph[i]->active = true;
            }
          }
        } // set_layout(start_x, width)


        // return a graph of ping data
        Dot** pings() {
            // Log.trace("getting pings");
            if (show_pinger && ping_timer->isExpired()) {
                // Serial.println("pinger: updating graph");
                // Serial.printf("graph: [%d,%d]\n", GRAPH_MIN, GRAPH_MAX);
                update_graph();
            }
            
            return &graph[0];
        } // Dot** pings()


        // returns the # pixels in the graph,
        // or 0 if not showing
        int npings() {
          if (show_pinger) {
            return MATRIX_Y;
          } else {
            return 0;
          }
        } // npings()
};


#endif
