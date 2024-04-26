#ifndef RATE_LIMITED_LOOP_HPP
#define RATE_LIMITED_LOOP_HPP

#include <functional>

class RateLimitedLoop {
public:
  void start(double update_rate_hz,
             const std::function<void()> &rate_limited_func,
             const std::function<bool()> &termination_condition_func);
};

#endif // RATE_LIMITED_LOOP_HPP
