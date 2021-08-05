#include <iostream>
#include <thread>

#include "exec.hpp"

namespace kuro = exec;

auto thread_id() { return std::this_thread::get_id(); }

auto count(size_t n, double interval) -> kuro::Task<double> {
  std::cout << thread_id() << " start: " << interval << std::endl;
  auto start = std::chrono::high_resolution_clock::now();
  for (size_t i = 0; i < n; i += 1) {
    co_await kuro::sleep_for(interval);
    std::cout << thread_id() << " slept: " << interval << std::endl;
  }
  auto elapsed = std::chrono::high_resolution_clock::now() - start;
  co_return elapsed.count() * 1e-9;
}

auto run() -> kuro::Task<double> {
  auto elapsed = co_await count(3, 0.2);
  co_return elapsed;
}

auto main() -> int {
  auto total = kuro::event_loop::run(run());
  std::cout << total << "\n";
}
