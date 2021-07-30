#pragma once

#include <sys/timerfd.h>
#include <unistd.h>

#include <chrono>
#include <coroutine>

#include "event_loop.hpp"
#include "unique_fd.hpp"

namespace kuro {

class duration {
 public:
  duration(double d) {
    m_sec = long(d);
    m_nanosec = long((d - long(d)) * 1e9);
  }

  explicit operator timespec() const {
    return timespec{.tv_sec = m_sec, .tv_nsec = m_nanosec};
  }

 private:
  long m_sec;
  long m_nanosec;
};

class sleep_for {
 public:
  sleep_for(duration d)
      : m_fd(timerfd_create(CLOCK_MONOTONIC, 0)),
        m_time(static_cast<timespec>(d)) {}

  bool await_ready() const noexcept {
    return m_time.tv_sec < 0 || (m_time.tv_sec == 0 && m_time.tv_nsec == 0);
  }

  void await_resume() const noexcept {}

  void await_suspend(std::coroutine_handle<> handle) noexcept {
    itimerspec tspec{.it_value = m_time};
    timerfd_settime(m_fd.get(), 0, &tspec, nullptr);
    event_loop::add_reader(m_fd.get(), handle);
  }

 private:
  detail::unique_fd m_fd;
  timespec m_time;
};

}  // namespace kuro
