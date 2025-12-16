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

void FixedFrequencyLoop::start(const std::function<void(double)> &rate_limited_func,
                               const std::function<bool()> &termination_condition_func,
                               std::optional<std::function<void(IterationStats)>> loop_stats_function) {

    constexpr std::size_t max_history_size = 500; // Adjust as needed

    // NOTE: os sleep precision is much worse than one nanosecond, therefore we don't even care about fractional
    // nanoseconds, and so we can do this where this is duration of an unsigned int number of nanoseconds
    // NOTE: originally we used this:
    // std::chrono::duration<double> period(1.0 / max_update_rate_hz);
    // but after running a comparison test I determined that this one was slightly better and I believe it's due to
    // drift that occurs when you use floating point vs this which is an integer
    auto period_ns = std::chrono::nanoseconds{static_cast<long long>(1'000'000'000.0 / max_update_rate_hz)};

    loop_start_time = std::chrono::steady_clock::now();
    time_at_start_of_last_iteration = loop_start_time;

    double total_time = 0.0;

    bool should_keep_running = true;

    while (should_keep_running) {
        GlobalLogSection _("ffl while loop", log_mode);
        global_logger->info("iteration number: {}", iteration_count);

        // NOTE: recomputing this every time in case update rate changes, in general its over doing it a lot
        period_ns = std::chrono::nanoseconds{static_cast<long long>(1'000'000'000.0 / max_update_rate_hz)};
        auto period_sec = std::chrono::duration<double>(period_ns);

        auto current_time = std::chrono::steady_clock::now();
        auto elapsed = current_time - time_at_start_of_last_iteration;
        time_at_start_of_last_iteration = current_time;

        double measured_period = std::chrono::duration<double>(elapsed).count();
        double measured_period_delta = measured_period - period_sec.count();

        total_time += measured_period;
        ++num_periods_for_next_iteration;

        // TODO: should we force measured_period here?
        rate_limited_func(measured_period);
        average_fps.add_sample(1.0 / measured_period);

        if (loop_stats_function.has_value()) {
            // NOTE: i think a lot of this is deprecated
            IterationStats stats;
            stats.time_at_start_of_iteration = std::chrono::duration<double>(current_time - loop_start_time).count();
            stats.measured_period = measured_period;
            stats.measured_frequency_hz = 1 / stats.measured_period;
            stats.requested_period = period_sec.count();
            stats.measured_period_delta_wrt_requested_period = measured_period_delta;
            stats.sleeping_until =
                stats.time_at_start_of_iteration + period_sec.count() * num_periods_for_next_iteration;

            if (iteration_stats_history.size() >= max_history_size) {
                iteration_stats_history.pop_front();
            }
            iteration_stats_history.push_back(stats);

            (*loop_stats_function)(get_average_loop_stats());
        }

        if (operation_mode == OperationMode::fixed_frequency) {
            auto time_of_next_iteration = loop_start_time + period_ns * num_periods_for_next_iteration;

            // the following code waits until the next time we shoudl tick

            if (wait_strategy == WaitStrategy::sleep) {
                GlobalLogSection _("sleep", log_mode);

                auto now = std::chrono::steady_clock::now();
                if (time_of_next_iteration > now) {
                    std::this_thread::sleep_until(time_of_next_iteration);
                } // else we don't sleep because we don't need to.
            }

            if (wait_strategy == WaitStrategy::busy_wait) {
                // GlobalLogSection _("busy waiting", logging_enabled);
                while (std::chrono::steady_clock::now() < time_of_next_iteration) {
                    std::this_thread::yield();
                }
            }
        }

        should_keep_running = !termination_condition_func();
        iteration_count++;
    }
}

void FixedFrequencyLoop::set_max_update_rate_hz(double max_update_rate_hz) {
    // TODO: maybe create a reset function?
    this->max_update_rate_hz = max_update_rate_hz;
    reset();
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

void FixedFrequencyLoop ::reset() {
    loop_start_time = std::chrono::steady_clock::now();
    time_at_start_of_last_iteration = std::chrono::steady_clock::now();
    num_periods_for_next_iteration = 0;
}
