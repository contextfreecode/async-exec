#include <iostream>

#include "exec.hpp"

namespace kuro = exec;

auto run() -> kuro::Task<double> {
  auto start = std::chrono::high_resolution_clock::now();
  co_await kuro::sleep_for(0.6);
  auto elapsed = std::chrono::high_resolution_clock::now() - start;
  co_return elapsed.count() * 1e-9;
}

auto main() -> int {
  auto total = kuro::event_loop::run(run());
  std::cout << total << "\n";
}
