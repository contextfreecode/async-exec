#pragma once

#include <chrono>
#include <coroutine>
#include <cstdint>
#include <vector>

namespace exec {

using TimePoint = std::chrono::time_point<std::chrono::steady_clock>;

struct SleepHandle {
  TimePoint end;
  std::coroutine_handle<> handle;
};

auto sleeps = std::vector<SleepHandle>{};

auto sleep_ready(TimePoint end) -> bool {
  return std::chrono::steady_clock::now() >= end;
}

template <typename Value>
struct Task {
  struct promise_type {
    Value value;

    auto get_return_object() -> std::coroutine_handle<promise_type> {
      return std::coroutine_handle<promise_type>::from_promise(*this);
    }

    auto final_suspend() noexcept -> std::suspend_never { return {}; }
    auto initial_suspend() -> std::suspend_never { return {}; }
    auto return_value(Value value) -> void { this->value = value; }
    auto unhandled_exception() -> void {}
  };

  std::coroutine_handle<promise_type> handle;
};

struct Sleep {
  TimePoint end;

  auto await_ready() -> bool { return sleep_ready(end); }
  auto await_resume() {}

  auto await_suspend(std::coroutine_handle<> handle) {
    sleeps.push_back({.end = end, .handle = handle});
  }
};

auto sleep_for(double seconds) -> Sleep {
  using namespace std;
  auto duration = chrono::nanoseconds(int64_t(seconds * 1e9));
  return {.end = chrono::steady_clock::now() + duration};
}

namespace event_loop {

template <typename Value>
auto run(const Task<Value>& root) -> Value {
  while (sleeps.size()) {
    for (auto sleep = sleeps.begin(); sleep < sleeps.end(); sleep += 1) {
      if (sleep_ready(sleep->end)) {
        sleep->handle.resume();
        sleeps.erase(sleep);
        // With invalidated iters, just sloppily wait for the next pass.
        break;
      }
    }
  }
  return root.handle.promise().value;
}

}  // namespace event_loop

}  // namespace exec
