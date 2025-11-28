#ifndef RATE_LIMITED_LOOP_HPP
#define RATE_LIMITED_LOOP_HPP

#include <deque>
#include <functional>
#include <ostream>
#include "sbpt_generated_includes.hpp"

class IterationStats {
  public:
    double time_at_start_of_iteration = 0.0;
    double measured_period = 0.0;
    double measured_frequency_hz = 0.0;
    double requested_period = 0.0; // The requested period, i.e., the target period (1.0 / update_rate_hz)
    double measured_period_delta_wrt_requested_period = 0.0;
    double sleeping_until = 0.0;

    friend std::ostream &operator<<(std::ostream &os, const IterationStats &stats) {
        os << "Start Time: " << stats.time_at_start_of_iteration << " s, "
           << "Measured Period: " << stats.measured_period << " s, "
           << "Measured Frequency Hz: " << stats.measured_frequency_hz << " Hz, "
           << "Requested Period: " << stats.requested_period << " s, "
           << "Measured Period Delta: " << stats.measured_period_delta_wrt_requested_period << " s, "
           << "Sleeping Until: " << stats.sleeping_until << " s";
        return os;
    }
};

class FixedFrequencyLoop {
  public:
    double max_update_rate_hz;
    FixedFrequencyLoop(double max_update_rate_hz = 60)
        : max_update_rate_hz(max_update_rate_hz), iteration_stats_history(1000) {};

    bool logging_enabled = false;
    unsigned int iteration_count = 0;

    // NOTE: this generalizes a while loop that runs at a fixed frequency, addtionally if you want to know about the
    // statistics of the loop you can provide a loop stats function which will receive some averaged iteration stats on
    // every tick
    void start(
        const std::function<void(double)> &rate_limited_func, const std::function<bool()> &termination_condition_func,
        std::function<void(IterationStats)> loop_stats_function = [](IterationStats is) {});

    std::deque<IterationStats> iteration_stats_history;

    IterationStats get_average_loop_stats();

    // NOTE: we allow the user to disable the rate limiting if they want
    bool rate_limiter_enabled = true;

    // NOTE: this is unused
    Stopwatch stopwatch;
};

#endif // RATE_LIMITED_LOOP_HPP
