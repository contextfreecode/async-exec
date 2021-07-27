#include <chrono>
#include <cstdint>
#include <iostream>
#include <unifex/linux/io_epoll_context.hpp>
#include <unifex/sync_wait.hpp>
#include <unifex/task.hpp>
#include <unifex/timed_single_thread_context.hpp>
#include <unifex/when_all.hpp>

auto thread_id() { return std::this_thread::get_id(); }

struct EPollContext {
  ~EPollContext() {
    stop_source.request_stop();
    thread.join();
  }

  auto get_scheduler() { return context.get_scheduler(); }

private:
  unifex::linuxos::io_epoll_context context;
  unifex::inplace_stop_source stop_source;
  std::thread thread{[&] { context.run(stop_source.get_token()); }};
};

template <typename Scheduler>
requires unifex::scheduler<Scheduler>
auto count(Scheduler scheduler, int n, double interval)
    -> unifex::task<double> {
  auto start = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::nanoseconds(std::int64_t(interval * 1e9));
  for (size_t i = 0; i < n; i += 1) {
    // co_await scheduler.schedule_at(unifex::now(scheduler) + duration);
    co_await unifex::schedule_after(scheduler, duration);
    std::cout << thread_id() << " slept: " << interval << std::endl;
  }
  auto elapsed = std::chrono::high_resolution_clock::now() - start;
  co_return elapsed.count() * 1e-9;
}

template <typename Value>
auto get(std::variant<std::tuple<Value>> wrapped) -> Value {
  return std::get<0>(std::get<std::tuple<Value>>(wrapped));
}

template <typename Scheduler>
requires unifex::scheduler<Scheduler>
auto run(Scheduler scheduler) -> unifex::task<double> {
  std::cout << thread_id() << " begin" << std::endl;
  auto task1 = count(scheduler, 2, 1.0);
  auto task2 = count(scheduler, 3, 0.6);
  // See also: https://github.com/facebookexperimental/libunifex/issues/251
  auto [elapsed1, elapsed2] =
      co_await unifex::when_all(std::move(task1), std::move(task2));
  std::cout << thread_id() << " done" << std::endl;
  co_return get(elapsed1) + get(elapsed2);
}

auto main() -> int {
  // auto context = EPollContext{};
  auto context = unifex::timed_single_thread_context{};
  auto scheduler = context.get_scheduler();
  auto task = run(scheduler);
  std::cout << sizeof(task) << std::endl;
  auto total = unifex::sync_wait(std::move(task));
  std::cout << thread_id() << " total: " << *total << "\n";
}
