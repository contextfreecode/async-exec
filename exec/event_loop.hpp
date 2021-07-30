#pragma once

#include <signal.h>
#include <sys/epoll.h>
#include <sys/signalfd.h>
#include <unistd.h>

#include <coroutine>
#include <functional>
#include <unordered_map>
#include <vector>

#include "promise.hpp"
#include "task.hpp"
#include "task_executor.hpp"
#include "unique_fd.hpp"
#include "util.hpp"

namespace kuro {

class event_loop {
 public:
  template <typename T>
  static T run(task<T> root_task) {
    auto exec = [](task<T>& t) -> detail::task_executor_return<T> {
      co_return co_await t;
    }(root_task);
    io_loop(root_task);
    if constexpr (std::is_void_v<T>) {
      exec.handle.destroy();
    } else {
      T result = exec.handle.promise().result();
      exec.handle.destroy();
      return result;
    }
  }

  static void add_reader(int fd, std::coroutine_handle<> handle) {
    epoll_event ev{.events = EPOLLIN, .data = {.fd = fd}};
    epoll_ctl(instance().m_epoll_fd.get(), EPOLL_CTL_ADD, fd, &ev);
    instance().m_handles[fd] = handle;
  }

  static void remove_fd(int fd) {
    instance().m_handles.erase(fd);
    epoll_ctl(instance().m_epoll_fd.get(), EPOLL_CTL_DEL, fd, nullptr);
  }

 private:
  event_loop() {
    m_epoll_fd = epoll_create1(0);
    m_events.resize(32);
  }

  template <typename T>
  static void io_loop(const task<T>& root_task) {
    while (!root_task.done()) {
      int n_ev =
          epoll_wait(instance().m_epoll_fd.get(), instance().m_events.data(),
                     instance().m_events.size(), -1);
      for (int i = 0; i < n_ev; ++i) {
        int fd = instance().m_events[i].data.fd;
        auto h = instance().m_handles[fd];
        remove_fd(fd);
        h.resume();
      }
    }
  }

  static event_loop& instance() {
    static event_loop inst;
    return inst;
  }

  detail::unique_fd m_epoll_fd;
  std::vector<epoll_event> m_events;
  std::unordered_map<int, std::coroutine_handle<>> m_handles;
};

}  // namespace kuro
