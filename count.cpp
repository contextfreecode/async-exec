#include <chrono>
#include <iostream>
#include <kuro/kuro.hpp>
#include <thread>

template <typename... Values>
auto report(Values... values) {
  std::cout << std::this_thread::get_id();
  ((std::cout << " " << values), ...);
  std::cout << std::endl;
}

// #include "exec/kuro.hpp"
// #include "exec.hpp"

// namespace kuro = exec;

auto count(size_t n, double interval) -> kuro::task<double> {
  report("before loop", interval);
  auto start = std::chrono::high_resolution_clock::now();
  for (size_t i = 0; i < n; i += 1) {
    co_await kuro::sleep_for(interval);
    report("slept", interval);
  }
  auto elapsed = std::chrono::high_resolution_clock::now() - start;
  co_return elapsed.count() * 1e-9;
}

auto run() -> kuro::task<double> {
  report("begin");
  auto task1 = count(2, 1.0);
  auto task2 = count(3, 0.6);
  report("count size:", sizeof(task1));
  auto [elapsed1, elapsed2] = co_await kuro::gather(task1, task2);
  report("end");
  co_return elapsed1 + elapsed2;
}

auto main() -> int {
  auto total = kuro::event_loop::run(run());
  report("total:", total);
}
