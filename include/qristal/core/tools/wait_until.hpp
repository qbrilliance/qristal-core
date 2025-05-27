#pragma once

#include <chrono>
#include <concepts>
#include <thread>
#include <type_traits>

namespace qristal {

// Concepts
template <typename T>
concept CallableReturnsBool = requires(T t) {
  { t() } -> std::same_as<bool>;
};

template <typename T>
concept Duration =
    std::is_same_v<T,
                   std::chrono::duration<typename T::rep, typename T::period>>;

/**
 * @brief Repeatedly runs a callable until a its return value is true or the
 * timeout has been reached.
 *
 * @tparam TCallable A callable function that returns a bool
 * @tparam TDuration The std::chrono::duration timeout
 * @param callback The callback to repeatedly run until it returns true
 * @param timeout The maximum time to wait for the callback to return true
 * @return true If the callback returns true within the timeout
 * @return false If timeout has elapsed before the callback returns true
 */
template <CallableReturnsBool TCallable, Duration TDuration>
bool wait_until(TCallable &&callback, TDuration timeout) {
  int message_ready = 0;
  int counter = 0;
  using clock = std::chrono::high_resolution_clock;
  using namespace std::chrono_literals;
  std::chrono::time_point<clock> start_time = clock::now();
  std::chrono::time_point<clock> stop_time = start_time + timeout;
  bool timed_out = false;
  while (!callback() && !(timed_out = clock::now() > stop_time)) {
    std::this_thread::sleep_for(5us);
  }

  if (timed_out) {
    return false;
  }

  return true;
}

} // namespace qristal
