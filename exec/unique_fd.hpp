#pragma once

#include <system_error>
#include <unistd.h>

namespace kuro::detail {

inline void check_error(int ret_val) {
  if (ret_val == -1) {
    throw std::system_error(errno, std::system_category());
  }
}

class unique_fd {
 public:
  unique_fd() : m_fd(-1) {}
  explicit unique_fd(int fd) : m_fd(fd) {}
  unique_fd(const unique_fd&) = delete;
  unique_fd& operator=(const unique_fd&) = delete;
  unique_fd(unique_fd&& other) {
    m_fd = other.m_fd;
    other.m_fd = -1;
  }
  unique_fd& operator=(unique_fd&& other) {
    if (m_fd != other.m_fd) {
      if (m_fd != -1) {
        detail::check_error(close(m_fd));
      }
      m_fd = other.m_fd;
      other.m_fd = -1;
    }
    return *this;
  }
  ~unique_fd() { reset(); }

  explicit operator bool() const noexcept { return m_fd != -1; }
  unique_fd& operator=(int fd) {
    m_fd = fd;
    return *this;
  }
  int get() const noexcept { return m_fd; }
  void reset() {
    if (m_fd != -1) {
      detail::check_error(close(m_fd));
    }
  }

 private:
  int m_fd;
};

}  // namespace kuro::detail
