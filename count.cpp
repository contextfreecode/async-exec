#include <chrono>
#include <iostream>
#include <kuro/kuro.hpp>
#include <thread>
#include <vector>

// #include "exec.hpp"

// namespace kuro = exec;

auto thread_id() { return std::this_thread::get_id(); }

auto count(size_t n, double interval) -> kuro::task<double> {
  auto start = std::chrono::high_resolution_clock::now();
  for (size_t i = 0; i < n; i += 1) {
    co_await kuro::sleep_for(interval);
    std::cout << thread_id() << " slept: " << interval << std::endl;
  }
  auto elapsed = std::chrono::high_resolution_clock::now() - start;
  co_return elapsed.count() * 1e-9;
}

auto run() -> kuro::task<double> {
  std::cout << thread_id() << " begin" << std::endl;
  auto task1 = count(2, 1.0);
  auto task2 = count(3, 0.6);
  std::cout << thread_id() << " size " << sizeof(task1) << std::endl;
  auto [elapsed1, elapsed2] = co_await kuro::gather(task1, task2);
  std::cout << thread_id() << " end" << std::endl;
  co_return elapsed1 + elapsed2;
}

auto main() -> int {
  auto total = kuro::event_loop::run(run());
  std::cout << thread_id() << " total: " << total << "\n";
}
