#ifndef RATE_LIMITED_LOOP_HPP
#define RATE_LIMITED_LOOP_HPP

#include <deque>
#include <functional>
#include <optional>
#include <ostream>
#include <chrono>
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
  public: // member variables
    enum class OperationMode {
        fixed_frequency,
        as_fast_as_possible,
    };

    enum class WaitStrategy {
        sleep,
        busy_wait,
        // hybrid, // sleep until close to target, then spin (not implemented yet)
    };

    OperationMode operation_mode;
    WaitStrategy wait_strategy;
    LogSection::LogMode log_mode = LogSection::LogMode::disable;

    // NOTE: we allow the user to disable the rate limiting if they want
    bool rate_limiter_enabled = true;

  private: // member variables
    std::chrono::time_point<std::chrono::steady_clock> loop_start_time;
    std::chrono::time_point<std::chrono::steady_clock> time_at_start_of_last_iteration;
    unsigned int num_periods_for_next_iteration = 0;

    double max_update_rate_hz;

  public: // member functions
    FixedFrequencyLoop(double max_update_rate_hz = 60, WaitStrategy wait_strategy = WaitStrategy::sleep,
                       OperationMode operation_mode = OperationMode::fixed_frequency)
        : max_update_rate_hz(max_update_rate_hz), wait_strategy(wait_strategy), operation_mode(operation_mode),
          iteration_stats_history(1000) {};

    unsigned int iteration_count = 0;

    // NOTE: this generalizes a while loop that runs at a fixed frequency, addtionally if you want to know about the
    // statistics of the loop you can provide a loop stats function which will receive some averaged iteration stats on
    // every tick
    void start(const std::function<void(double)> &rate_limited_func,
               const std::function<bool()> &termination_condition_func,
               std::optional<std::function<void(IterationStats)>> loop_stats_function = std::nullopt);

    math_utils::SimpleMovingAverage average_fps{100};

    std::deque<IterationStats> iteration_stats_history;

    void set_max_update_rate_hz(double max_update_rate_hz);
    void set_operation_mode(OperationMode operation_mode) {
        this->operation_mode = operation_mode;
        reset();
    }

    IterationStats get_average_loop_stats();

  private: // member functions
    void reset();
};

#endif // RATE_LIMITED_LOOP_HPP
