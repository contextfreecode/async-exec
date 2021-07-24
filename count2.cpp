#include <chrono>
#include <iostream>
#include <unifex/linux/io_epoll_context.hpp>
#include <unifex/scheduler_concepts.hpp>
#include <unifex/timed_single_thread_context.hpp>
// #include <unifex/static_thread_pool.hpp>
#include <unifex/sync_wait.hpp>
#include <unifex/task.hpp>
#include <unifex/when_all.hpp>

template <typename Scheduler>
requires unifex::scheduler<Scheduler>
auto count(Scheduler scheduler, int n, double interval) -> unifex::task<void> {
  using namespace std::chrono_literals;
  // auto duration = std::chrono::duration<double>(interval);
  auto duration = std::chrono::nanoseconds(int64_t(interval * 1e9));
  for (size_t i = 0; i < n; i += 1) {
    // co_await unifex::schedule_after(scheduler, 500ms);
    co_await unifex::schedule_after(scheduler, duration);
    std::cout << interval << " seconds" << std::endl;
  }
  std::cout << "done" << std::endl;
}

template <typename Scheduler>
requires unifex::scheduler<Scheduler>
auto run(Scheduler scheduler) -> unifex::task<void> {
  auto task1 = count(scheduler, 2, 1.0);
  auto task2 = count(scheduler, 3, 0.6);
  std::cout << sizeof(task1) << std::endl;
  // co_await unifex::when_all(task1, task2);
  co_await task1;
}

auto main() -> int {
  auto ctx = unifex::timed_single_thread_context{};
  // auto ctx = unifex::static_thread_pool{};
  // auto ctx = unifex::linuxos::io_epoll_context{};
  // unifex::inplace_stop_source stop_source;
  // std::thread thread{[&] { ctx.run(stop_source.get_token()); }};
  // unifex::scope_guard stop_on_exit = [&]() noexcept {
  //   stop_source.request_stop();
  //   thread.join();
  // };
  auto scheduler = ctx.get_scheduler();
  auto task = count(scheduler, 2, 1.0);
  // auto task = run(scheduler);
  std::cout << sizeof(task) << std::endl;
  unifex::sync_wait(std::move(task));
  // unifex::sync_wait(std::move(task2));
  std::cout << "really done" << std::endl;
}
