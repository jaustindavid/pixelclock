#pragma once

/*
 * a simple Statistics engine
 *
 * usage:
 *    Statistics stats(10); // 10MA
 *    stats.add(value);
 *    stats.read(); // returns the average values from the last 10
 */
class Statistics {
  private:
    int ma_target;
    int n_events;
    int sum;
    int avg;

  public:
    Statistics(int n) {
      ma_target = n;
      n_events = 0;
      sum = 0;
      avg = 0;
    }


    void add(int value) {
      sum += value;
      n_events ++;
      if (n_events > ma_target) {
        sum -= avg;
        n_events = ma_target;
      }
      avg = sum/n_events;
    }


    int read() {
      return avg;
    }
};


class TimedStatistics {
  private:
    Statistics *stats;
    Stopwatch *stopwatch;

  public:
    TimedStatistics(int n) {
      stats = new Statistics(n);
      stopwatch = new Stopwatch();
    }


    ~TimedStatistics() {
      delete stats;
      delete stopwatch;
    }


    void start() {
      stopwatch->start();
    }


    void stop() {
      stopwatch->stop();
      stats->add(stopwatch->read());
      stopwatch->reset();
    }


    int read() {
      return stats->read();
    }
};
