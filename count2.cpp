#include <chrono>
#include <iostream>
#include <unifex/linux/io_epoll_context.hpp>
#include <unifex/scheduler_concepts.hpp>
#include <unifex/sync_wait.hpp>
#include <unifex/task.hpp>

template<typename Scheduler>
requires unifex::scheduler<Scheduler>
auto count(Scheduler& scheduler) -> unifex::task<void> {
    using namespace std::chrono_literals;
    co_await scheduler.schedule_at(unifex::now(scheduler) + 100ms);
    std::cout << "done" << std::endl;
}

auto main() -> int {
    auto ctx = unifex::linuxos::io_epoll_context{};
    auto scheduler = ctx.get_scheduler();
    auto task = count(scheduler);
    std::cout << sizeof(task) << std::endl;
    unifex::sync_wait(std::move(task));
}
