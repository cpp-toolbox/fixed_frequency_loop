#ifndef RATE_LIMITED_LOOP_HPP
#define RATE_LIMITED_LOOP_HPP

#include <functional>
#include <ostream>
#include "sbpt_generated_includes.hpp"

class IterationStats {
  public:
    double time_at_start_of_iteration = 0.0;
    double measured_period = 0.0;
    double requested_period = 0.0; // The requested period, i.e., the target period (1.0 / update_rate_hz)
    double measured_period_delta = 0.0;
    double sleeping_until = 0.0;

    friend std::ostream &operator<<(std::ostream &os, const IterationStats &stats) {
        os << "Start Time: " << stats.time_at_start_of_iteration << " s, "
           << "Measured Period: " << stats.measured_period << " s, "
           << "Requested Period: " << stats.requested_period << " s, "
           << "Measured Period Delta: " << stats.measured_period_delta << " s, "
           << "Sleeping Until: " << stats.sleeping_until << " s";
        return os;
    }
};

class FixedFrequencyLoop {
  public:
    void start(double update_rate_hz, const std::function<void(double)> &rate_limited_func,
               const std::function<bool()> &termination_condition_func, bool logging = false);

    LimitedVector<IterationStats> iteration_stats_history;

    FixedFrequencyLoop() : iteration_stats_history(1000) {};

    Stopwatch stopwatch;
};

#endif // RATE_LIMITED_LOOP_HPP
