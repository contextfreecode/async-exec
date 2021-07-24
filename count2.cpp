#include <chrono>
#include <iostream>
#include <unifex/linux/io_epoll_context.hpp>
#include <unifex/sync_wait.hpp>
#include <unifex/task.hpp>
#include <unifex/timed_single_thread_context.hpp>
#include <unifex/when_all.hpp>

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
auto count(Scheduler scheduler, int n, double interval) -> unifex::task<void> {
  auto duration = std::chrono::nanoseconds(int64_t(interval * 1e9));
  for (size_t i = 0; i < n; i += 1) {
    // co_await scheduler.schedule_at(unifex::now(scheduler) + duration);
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
  co_await unifex::when_all(std::move(task1), std::move(task2));
  // See also: https://github.com/facebookexperimental/libunifex/issues/251
}

auto main() -> int {
  // auto context = EPollContext{};
  auto context = unifex::timed_single_thread_context{};
  auto scheduler = context.get_scheduler();
  auto task = run(scheduler);
  std::cout << sizeof(task) << std::endl;
  unifex::sync_wait(std::move(task));
  std::cout << "really done" << std::endl;
}
