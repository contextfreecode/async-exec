#pragma once

#include <chrono>
#include <coroutine>
#include <cstdint>

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
  using namespace std;
  auto duration = chrono::duration<int64_t>(int64_t(seconds * 1e-9));
  return {.end = chrono::steady_clock::now() + duration};
}

}  // namespace exec
