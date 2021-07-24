#include <chrono>
#include <unifex/sync_wait.hpp>
#include <unifex/task.hpp>
#include <unifex/timed_single_thread_context.hpp>
#include <unifex/when_all.hpp>

template <typename Scheduler>
requires unifex::scheduler<Scheduler>
auto sleep(Scheduler scheduler) -> unifex::task<void> {
  using namespace std::chrono_literals;
  co_await unifex::schedule_after(scheduler, 500ms);
}

template <typename Scheduler>
requires unifex::scheduler<Scheduler>
auto await_sleep(Scheduler scheduler) -> unifex::task<void> {
  co_await sleep(scheduler);
  co_await unifex::when_all(sleep(scheduler), sleep(scheduler));
}

auto main() -> int {
  unifex::timed_single_thread_context context;
  auto scheduler = context.get_scheduler();
  // auto task = sleep(scheduler);
  auto task = await_sleep(scheduler);
  unifex::sync_wait(std::move(task));
}
