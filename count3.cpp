#include <coroutine>
#include <iostream>

template <typename Value>
struct Task {
  struct promise_type {
    auto get_return_object() -> std::coroutine_handle<promise_type> {
      return std::coroutine_handle<promise_type>::from_promise(*this);
    }

    auto final_suspend() noexcept -> std::suspend_never { return {}; }

    auto initial_suspend() -> std::suspend_never { return {}; }

    auto return_value(Value value) -> void { this->value = value; }

    auto unhandled_exception() -> void {}

    Value value;
  };

  std::coroutine_handle<promise_type> handle;
};

struct sleep_for {
  sleep_for(double seconds) {
    //
  }

  auto await_ready() -> bool { return true; }

  auto await_resume() {}

  auto await_suspend(std::coroutine_handle<> handle) {}
};

auto thing() -> Task<int> {
  co_await sleep_for(0.6);
  co_return 3;
}

auto main() -> int {
  // auto hi = co_await thing();
  auto hi = thing();
  std::cout << hi.handle.promise().value << "\n";
}
