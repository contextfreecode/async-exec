#include <coroutine>

template <typename Value>
struct Task {
  struct promise_type {
    //
  };
  //
};

auto thing() -> Task<int> {
  co_return 3;
}

auto main() -> int {
  // auto hi = co_await thing();
  auto hi = thing();
}
