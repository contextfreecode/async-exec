#include <chrono>
#include <iostream>
#include <kuro/kuro.hpp>
#include <vector>

auto count(size_t n, double interval) -> kuro::task<void> {
  for (size_t i = 0; i < n; i += 1) {
    co_await kuro::sleep_for(interval);
    std::cout << interval << " seconds" << std::endl;
  }
}

auto run() -> kuro::task<void> {
  std::cout << "begin" << std::endl;
  auto task1 = count(2, 1.0);
  auto task2 = count(3, 0.6);
  std::cout << "size " << sizeof(task1) << std::endl;
  co_await kuro::gather(task1, task2);
  std::cout << "end" << std::endl;
}

auto main() -> int { kuro::event_loop::run(run()); }
