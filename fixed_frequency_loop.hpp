#ifndef RATE_LIMITED_LOOP_HPP
#define RATE_LIMITED_LOOP_HPP

#include <functional>
#include "sbpt_generated_includes.hpp"

class FixedFrequencyLoop {
public:
    void start(double update_rate_hz, const std::function<void(double)> &rate_limited_func,
               const std::function<bool()> &termination_condition_func);

    Stopwatch stopwatch;
};

#endif // RATE_LIMITED_LOOP_HPP
