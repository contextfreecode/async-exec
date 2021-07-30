#include <coroutine>
#include <iostream>

template <typename Value>
struct Task {
  struct promise_type {
    auto get_return_object() {
      return std::coroutine_handle<promise_type>::from_promise(*this);
    }
    auto final_suspend() noexcept -> std::suspend_never { return {}; }
    auto initial_suspend() -> std::suspend_never { return {}; }
    auto return_value(Value value) {
      this->value = value;
    }
    auto unhandled_exception() {}
    Value value;
  };

  std::coroutine_handle<promise_type> handle;
};

auto thing() -> Task<int> {
  co_return 3;
}

auto main() -> int {
  // auto hi = co_await thing();
  auto hi = thing();
  std::cout << hi.handle.promise().value << "\n";
}
