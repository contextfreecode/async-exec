#include <chrono>
#include <iostream>
#include <unifex/linux/io_epoll_context.hpp>
#include <unifex/scheduler_concepts.hpp>
#include <unifex/sync_wait.hpp>
#include <unifex/task.hpp>

template<typename Scheduler>
requires unifex::scheduler<Scheduler>
auto count(Scheduler& scheduler, int n, double interval) -> unifex::task<void> {
    auto duration = std::chrono::duration<double>(1.0);
    for (size_t i = 0; i < n; i += 1) {
        co_await scheduler.schedule_at(unifex::now(scheduler) + duration);
        std::cout << interval << " seconds" << std::endl;
    }
    std::cout << "done" << std::endl;
}

auto main() -> int {
    // using namespace std::chrono_literals;
    // unifex::linuxos::io_epoll_context ctx;
    auto ctx = unifex::linuxos::io_epoll_context{};
    unifex::inplace_stop_source stop_source;
    std::thread thread{[&] { ctx.run(stop_source.get_token()); }};
    unifex::scope_guard stop_on_exit = [&]() noexcept {
        stop_source.request_stop();
        thread.join();
    };
    auto scheduler = ctx.get_scheduler();
    // auto task = scheduler.schedule_at(unifex::now(scheduler) + 100ms);
    auto task = count(scheduler, 2, 1.0);
    std::cout << sizeof(task) << std::endl;
    unifex::sync_wait(std::move(task));
    std::cout << "really done" << std::endl;
}
