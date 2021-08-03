#include <iostream>

#include "exec.hpp"

namespace kuro = exec;

auto run() -> kuro::Task<double> {
  co_await kuro::sleep_for(0.6);
  co_return 3.5;
}

auto main() -> int {
  // auto hi = co_await thing();
  auto hi = run();
  std::cout << hi.handle.promise().value << "\n";
}
