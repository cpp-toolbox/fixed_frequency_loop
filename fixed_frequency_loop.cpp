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
                               const std::function<bool()> &termination_condition_func, bool logging) {
    const std::chrono::duration<double> period(1.0 / update_rate_hz);

    auto loop_start_time = std::chrono::steady_clock::now();
    auto time_at_start_of_last_iteration = loop_start_time;

    double total_time = 0.0;
    int count = 0;

    while (!termination_condition_func()) {
        auto current_time = std::chrono::steady_clock::now();
        auto elapsed = current_time - time_at_start_of_last_iteration;
        time_at_start_of_last_iteration = current_time;

        double measured_period = std::chrono::duration<double>(elapsed).count();
        double measured_period_delta = measured_period - period.count(); // Difference from expected period

        total_time += measured_period;
        ++count;

        rate_limited_func(measured_period);

        // Populate the iteration_stats_history with the current stats
        IterationStats stats;
        stats.time_at_start_of_iteration = std::chrono::duration<double>(current_time - loop_start_time).count();
        stats.measured_period = measured_period;
        stats.requested_period = period.count();             // Set the requested period (target period)
        stats.measured_period_delta = measured_period_delta; // Store the measured period delta
        stats.sleeping_until = stats.time_at_start_of_iteration + period.count() * count;
        iteration_stats_history.push_back(stats); // Add to history

        if (logging) {
            // Log each iteration's stats
            std::cout << stats << std::endl;

            // Calculate and log average stats if logging is enabled
            if (iteration_stats_history.size() > 1) {
                double total_measured_period = 0.0;
                double total_measured_period_delta = 0.0;
                for (const auto &iter_stats : iteration_stats_history) {
                    total_measured_period += iter_stats.measured_period;
                    total_measured_period_delta += iter_stats.measured_period_delta;
                }

                double avg_measured_period = total_measured_period / iteration_stats_history.size();
                double avg_measured_period_delta = total_measured_period_delta / iteration_stats_history.size();

                std::cout << "Average Measured Period: " << avg_measured_period << " s\n";
                std::cout << "Average Measured Period Delta: " << avg_measured_period_delta << " s\n";
            }
        }

        // But the idea with sleep_until is that if you do sleep_until(start_ts + interval * i); you'll always schedule
        // the next interval for the correct time you won't get a cascading error building up
        std::this_thread::sleep_until(loop_start_time + period * (count));
    }

    // Optional: Print the average frequency and period after the loop ends
    // double average_freq = count / total_time;
    // double average_period = total_time / count;
    // std::cout << "Average Frequency: " << average_freq << " Hz" << std::endl;
    // std::cout << "Average Period: " << average_period << " seconds" << std::endl;
}
