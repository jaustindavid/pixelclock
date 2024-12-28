#pragma once

/*
 * A class which holds and manages the "layout" of the display
 *   - different attributes like "weather" or "pinger"
 *   - proving an interface (Particle.function) for changing
 *   - persisting same to EEPROM
 * 
 * TODO:
 *   - layout tells pinger, weather, chef what to draw
 *     - (x, y) anchors & lengths
 *   - layout renders these to Display
 */

class Layout {
  private:

    struct layout_datum {
      byte version;
      bool pinger;
      bool weather;
      bool plan;
    };


    // reads pinger, weather, plan from EEPROM
    void read_eeprom() {
      struct layout_datum datum;

      EEPROM.get(LAYOUT_ADDY, datum);

      if (datum.version == 1) {
        show_pinger = datum.pinger;
        show_weather = datum.weather;
        show_plan = datum.plan;
      }

      // TODO: fix weather
      show_weather = false;
    } // void read_eeprom()


    // stores pinger, weather, plan
    void write_eeprom() {
      struct layout_datum datum = 
        { .version = 1, 
          .pinger = show_pinger, 
          .weather = show_weather, 
          .plan = show_plan
        };

      EEPROM.put(LAYOUT_ADDY, datum);
    } // write_eeprom()


    // used by Particle.function
    int toggle_pinger(String s) {
      show_pinger = !show_pinger;
      write_eeprom();
      return show_pinger ? 1 : 0;
    } // int toggle_pinger(s)


    // used by Particle.function
    int toggle_weather(String s) {
      show_weather = !show_weather;
      write_eeprom();
      return show_weather ? 1 : 0;
    } // int toggle_pinger(s)


    // used by Particle.function
    int toggle_plan(String s) {
      show_plan = !show_plan;
      write_eeprom();
      return show_plan ? 1 : 0;
    } // int toggle_plan(s)

  public:
    bool show_pinger, show_weather, show_plan;

    Layout() : show_pinger(false), 
               show_weather(false), 
               show_plan(false) {
    } // Layout()


    void setup() {
      read_eeprom();
    } // setup()


    // sets up the cloud functions
    void setup_cloud() {
      Particle.function("toggle_pinger", &Layout::toggle_pinger, this);
      Particle.function("toggle_weather", &Layout::toggle_weather, this);
      Particle.function("toggle_plan", &Layout::toggle_plan, this);
    } // setup_cloud()

}; // class Layout
