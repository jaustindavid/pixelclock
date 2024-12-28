#pragma once

/*
 * a placeholder for some "nite lite" code,
 * which basically disables animation and sets 
 * the clock to minimum brightness during a
 * window of time
 */


/*
 * nitetime
 *
 * 99:99 - 99:99 : disable
 * any other:
 *   start nitetime when hh:mm
 *   end nitetime when hh:mm
 */

typedef struct {
  byte hh;
  byte mm;
} hh_mm_struct_t;

hh_mm_struct_t start_nitetime, end_nitetime;
String nitetime_s;
bool is_nitetime;


bool valid_hhmm(hh_mm_struct_t sample) {
  return sample.hh >= 0
      && sample.hh <= 23
      && sample.mm >= 0
      && sample.mm <= 59;
}


int minnify(const byte hh, const byte mm) {
  return 60*hh + mm;
}


/*
 * 
 */

// ONLY WORKS ON THE BOUNDARY
// if no valid schedule, is_nitetime -> false
// otherwise, only change is_nitetime if the times match
void check_nitetime(byte hh, byte mm) {
  if (valid_hhmm(start_nitetime)
      && valid_hhmm(end_nitetime)) {
    if (start_nitetime.hh == hh
        && start_nitetime.mm == mm) {
      is_nitetime = true;
    }
    if (end_nitetime.hh == hh
        && end_nitetime.mm == mm) {
      is_nitetime = false;
    }
  } else {
    is_nitetime = false;
  }
}


void stringify_nitetime() {
  if (valid_hhmm(start_nitetime)
      && valid_hhmm(end_nitetime)) {
    nitetime_s = String::format("nitetime: %02d:%02d - %02d:%02d",
                                start_nitetime.hh, start_nitetime.mm,
                                end_nitetime.hh, end_nitetime.mm);
  } else {
    nitetime_s = "no nitetime schedule";
  }
} // stringify_nitetime()


// attempts to parse
hh_mm_struct_t parse_hhmm(String s) {
  hh_mm_struct_t ret = { .hh = 99, .mm = 99 };
  int i = s.indexOf(":");
  if (i != 0) {
    byte hh = s.toInt();
    s = s.substring(i+1);
    byte mm = s.toInt();
    ret.hh = hh;
    ret.mm = mm;
  }
  return ret;
} // hh_mm_struct_t parse_hhmm(s)


// parses "hh:mm - hh:mm"
int set_nitetime(String s) {
  hh_mm_struct_t start, end;
  start = parse_hhmm(s);
  int i = s.indexOf(" - ");
  s = s.substring(i+3);
  end = parse_hhmm(s);
  if (valid_hhmm(start)
      && valid_hhmm(end)) {
    start_nitetime = start;
    end_nitetime = end;
    stringify_nitetime();
    return 1;
  }
  return -1;
} // int set_nitetime(s)


void nitetime_setup_cloud() {
  static bool _cloud_setup_complete = false;

  // bail early?
  if (_cloud_setup_complete
      || ! Particle.connected()) {
    return;
  }

  Particle.function("set_nitetime", set_nitetime);
  Particle.variable("nitetime_schedule", nitetime_s);
} // nitetime_setup_cloud()


void nitetime_setup() {
  start_nitetime = { .hh = 99, .mm = 99 };
  end_nitetime = { .hh = 99, .mm = 99 };
  stringify_nitetime();
  is_nitetime = true;
  nitetime_setup_cloud();
} // nitetime_setup()
