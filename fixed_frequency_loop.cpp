#include "fixed_frequency_loop.hpp"
#include <chrono>
#include <thread>

/**
 * \brief a sleep based loop that runs a function at a fixed interval
 *
 * \note you probably want to run this in it's own thread so the sleep doesn't
 * pause your whole program
 *
 * \author cuppajoeman
 */

void FixedFrequencyLoop::start(double update_rate_hz, const std::function<void(double)> &rate_limited_func, const std::function<bool()> &termination_condition_func) {
        // 1/N seconds per iteration
        std::chrono::duration<double> update_period(1.0 / update_rate_hz);

        bool first_iteration = true;
        std::chrono::time_point<std::chrono::steady_clock, std::chrono::duration<double>> previous_time;

        while (!termination_condition_func()) {
            if (first_iteration) {
                previous_time = std::chrono::steady_clock::now();
                first_iteration = false;
            } else {
                this->stopwatch.press();
                std::chrono::time_point<std::chrono::steady_clock, std::chrono::duration<double>> current_time = std::chrono::steady_clock::now();
                std::chrono::duration<double> delta = current_time - previous_time;

                rate_limited_func(delta.count());

                std::chrono::time_point<std::chrono::steady_clock, std::chrono::duration<double>> end_time = std::chrono::steady_clock::now();
                std::chrono::duration<double> elapsed = end_time - current_time;
                auto sleep_duration = update_period - elapsed;

                if (sleep_duration > std::chrono::duration<double>::zero()) {
                    std::this_thread::sleep_for(sleep_duration);
                }

                previous_time = current_time;
            }
        }
    }
