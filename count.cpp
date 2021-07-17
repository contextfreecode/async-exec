#include <chrono>
#include <iostream>
#include <kuro/kuro.hpp>

auto run() -> kuro::task<void> {
    std::cout << "begin" << std::endl;
    co_await kuro::sleep_for(0.6);
    std::cout << "end" << std::endl;
}

auto main() -> int {
    kuro::event_loop::run(run());
}
