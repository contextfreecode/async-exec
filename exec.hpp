#pragma once

#include <chrono>
#include <coroutine>

namespace exec {

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
  std::chrono::time_point<std::chrono::steady_clock> end;

  auto await_ready() -> bool { return std::chrono::steady_clock::now() >= end; }
  auto await_resume() {}
  auto await_suspend(std::coroutine_handle<>) {}
};

auto sleep_for(double seconds) -> Sleep {
  return {.end = std::chrono::steady_clock::now() +
                 std::chrono::duration<std::int64_t>(
                     static_cast<std::int64_t>(seconds * 1e-9))};
}

}  // namespace exec
