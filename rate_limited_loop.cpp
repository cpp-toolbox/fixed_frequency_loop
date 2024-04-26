#include "rate_limited_loop.hpp"
#include <chrono>
#include <thread>

/**
 * \brief a sleep based loop that runs a function at a fixed interval
 *
 * \note you probably want to run this in it's own thread so the sleep doesn't
 * pause your whole program
 */
void RateLimitedLoop::start(
    double update_rate_hz, const std::function<void()> &rate_limited_func,
    const std::function<bool()> &termination_condition_func) {

  // 1/N seconds per iteration
  std::chrono::duration<double> update_period(1.0 / update_rate_hz);

  while (!termination_condition_func()) {
    rate_limited_func();

    std::chrono::time_point<std::chrono::steady_clock,
                            std::chrono::duration<double>>
        now = std::chrono::steady_clock::now();
    std::chrono::time_point<std::chrono::steady_clock,
                            std::chrono::duration<double>>
        wake_time = now + update_period;

    std::this_thread::sleep_until(wake_time);
  }
}
