#include "fixed_frequency_loop.hpp"
#include <chrono>
#include <thread>
#include <iostream>

/**
 * \brief a sleep based loop that runs a function at a fixed interval
 *
 * \note you probably want to run this in it's own thread so the sleep doesn't
 * pause your whole program, unless this is your "whole program"
 *
 * \author cuppajoeman
 */

void FixedFrequencyLoop::start(double update_rate_hz, const std::function<void(double)> &rate_limited_func,
                               const std::function<bool()> &termination_condition_func) {
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

        total_time += std::chrono::duration<double>(elapsed).count();
        ++count;

        rate_limited_func(measured_period);

        // But the idea with sleep_until is that if you do sleep_until(start_ts + interval * i); you'll always schedule
        // the next interval for the correct time you won't get a cascading error building up
        std::this_thread::sleep_until(loop_start_time + period * (count));
    }

    /*// Calculate the average frequency and period after the loop ends*/
    /*double average_freq = count / total_time;*/
    /*double average_period = total_time / count;*/
    /**/
    /*// Print the final results*/
    /*std::cout << "Average Frequency: " << average_freq << " Hz" << std::endl;*/
    /*std::cout << "Average Period: " << average_period << " seconds" << std::endl;*/
}
