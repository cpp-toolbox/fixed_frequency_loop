#include "fixed_frequency_loop.hpp"
#include <chrono>
#include <thread>
#include <iostream>

/**
 * \brief a sleep based loop that runs a function at a fixed interval
 *
 * \note you probably want to run this in its own thread so the sleep doesn't
 * pause your whole program, unless this is your "whole program"
 *
 * \author cuppajoeman
 */

void FixedFrequencyLoop::start(double update_rate_hz, const std::function<void(double)> &rate_limited_func,
                               const std::function<bool()> &termination_condition_func,
                               std::function<void(IterationStats)> loop_stats_function) {

    constexpr std::size_t max_history_size = 500; // Adjust as needed

    std::chrono::duration<double> period(1.0 / update_rate_hz);

    auto loop_start_time = std::chrono::steady_clock::now();
    auto time_at_start_of_last_iteration = loop_start_time;

    double total_time = 0.0;
    int count = 0;

    while (!termination_condition_func()) {

        // NOTE: reocmputing this every time in case update rate changes, in general its over doing it a lot
        period = std::chrono::duration<double>(1.0 / update_rate_hz);

        auto current_time = std::chrono::steady_clock::now();
        auto elapsed = current_time - time_at_start_of_last_iteration;
        time_at_start_of_last_iteration = current_time;

        double measured_period = std::chrono::duration<double>(elapsed).count();
        double measured_period_delta = measured_period - period.count();

        total_time += measured_period;
        ++count;

        rate_limited_func(measured_period);

        IterationStats stats;
        stats.time_at_start_of_iteration = std::chrono::duration<double>(current_time - loop_start_time).count();
        stats.measured_period = measured_period;
        stats.measured_frequency_hz = 1 / stats.measured_period;
        stats.requested_period = period.count();
        stats.measured_period_delta_wrt_requested_period = measured_period_delta;
        stats.sleeping_until = stats.time_at_start_of_iteration + period.count() * count;

        if (iteration_stats_history.size() >= max_history_size) {
            iteration_stats_history.pop_front(); // Assuming it's a deque or similar
        }
        iteration_stats_history.push_back(stats);

        loop_stats_function(get_average_loop_stats());

        if (rate_limiter_enabled)
            std::this_thread::sleep_until(loop_start_time + period * count);
    }
}

IterationStats FixedFrequencyLoop::get_average_loop_stats() {
    IterationStats avg_stats{};

    if (iteration_stats_history.empty())
        return avg_stats;

    double total_measured_period = 0.0;
    double total_requested_period = 0.0;
    double total_measured_period_delta = 0.0;
    double total_time_at_start = 0.0;
    double total_sleeping_until = 0.0;

    for (const auto &stats : iteration_stats_history) {
        total_measured_period += stats.measured_period;
        total_requested_period += stats.requested_period;
        total_measured_period_delta += stats.measured_period_delta_wrt_requested_period;
        total_time_at_start += stats.time_at_start_of_iteration;
        total_sleeping_until += stats.sleeping_until;
    }

    size_t count = iteration_stats_history.size();
    avg_stats.measured_period = total_measured_period / count;
    avg_stats.measured_frequency_hz = 1 / avg_stats.measured_period;
    avg_stats.requested_period = total_requested_period / count;
    avg_stats.measured_period_delta_wrt_requested_period = total_measured_period_delta / count;
    avg_stats.time_at_start_of_iteration = total_time_at_start / count;
    avg_stats.sleeping_until = total_sleeping_until / count;

    return avg_stats;
}
