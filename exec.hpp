#include <coroutine>
#include <exception>
#include <memory>

namespace exec {

namespace detail {

template <typename Promise> class unique_coroutine_handle {
public:
  unique_coroutine_handle() noexcept = default;

  unique_coroutine_handle(std::coroutine_handle<Promise> h)
      : m_ptr(h.address()) {}

  bool done() const { return handle().done(); }

  Promise &promise() const { return handle().promise(); }

  void resume() const { handle().resume(); }

  operator std::coroutine_handle<Promise>() const { return handle(); }

private:
  std::coroutine_handle<Promise> handle() const noexcept {
    return std::coroutine_handle<Promise>::from_address(m_ptr.get());
  }

  struct destroyer {
    constexpr destroyer() noexcept = default;
    void operator()(void *ptr) {
      std::coroutine_handle<Promise>::from_address(ptr).destroy();
    }
  };

  std::unique_ptr<void, destroyer> m_ptr;
};

template <typename T> class return_value_promise {
  enum class promise_state { empty, data, exception };

public:
  return_value_promise(){};
  ~return_value_promise() {
    if (m_state == promise_state::data) {
      m_data.~T();
    } else if (m_state == promise_state::exception) {
      m_except.~exception_ptr();
    }
  }
  void unhandled_exception() {
    m_except = std::current_exception();
    m_state = promise_state::exception;
  }

protected:
  T &get_data() {
    if (m_state == promise_state::data) {
      return m_data;
    } else if (m_state == promise_state::exception) {
      std::rethrow_exception(m_except);
    } else {
      throw std::exception{};
    }
  }

  void set_data(T data) {
    new (&m_data) T(std::move(data));
    m_state = promise_state::data;
  }

private:
  promise_state m_state = promise_state::empty;
  union {
    T m_data;
    std::exception_ptr m_except;
  };
};

template <typename T> class value_promise : public return_value_promise<T> {
public:
  T result() { return std::move(this->get_data()); }
  void return_value(T value) noexcept { this->set_data(std::move(value)); }
};

template <typename T>
class reference_promise : public return_value_promise<T *> {
public:
  T &result() { return *this->get_data(); }
  void return_value(T &value) noexcept { this->set_data(&value); }
};

class void_promise {
public:
  void result() const {
    if (m_except) {
      std::rethrow_exception(m_except);
    }
  }
  void return_void() noexcept {}
  void unhandled_exception() { m_except = std::current_exception(); }

private:
  std::exception_ptr m_except = nullptr;
};

template <typename T> struct base_promise { using type = value_promise<T>; };

template <typename T> struct base_promise<T &> {
  using type = reference_promise<T>;
};

template <> struct base_promise<void> { using type = void_promise; };

template <typename T> using base_promise_t = base_promise<T>::type;

} // namespace detail

template <typename T> class task {
public:
  class promise_type final : public detail::base_promise_t<T> {
  public:
    task<T> get_return_object() noexcept {
      return task(std::coroutine_handle<promise_type>::from_promise(*this));
    }

    std::suspend_always initial_suspend() const noexcept { return {}; }

    auto final_suspend() const noexcept {
      struct awaitable {
        bool await_ready() noexcept { return false; }
        std::coroutine_handle<>
        await_suspend(std::coroutine_handle<promise_type> handle) noexcept {
          auto continuation = handle.promise().m_continuation;
          if (continuation) {
            return continuation;
          }
          return std::noop_coroutine();
        }
        void await_resume() noexcept {}
      };
      return awaitable{};
    }

    void set_continuation(std::coroutine_handle<> continuation) {
      m_continuation = continuation;
    }

  private:
    std::coroutine_handle<> m_continuation;
  };

  auto operator co_await() {
    struct awaitable {
      awaitable(std::coroutine_handle<promise_type> handle)
          : m_handle(handle) {}

      T await_resume() { return m_handle.promise().result(); }

      bool await_ready() const noexcept { return m_handle.done(); }

      std::coroutine_handle<>
      await_suspend(std::coroutine_handle<> parent_handle) noexcept {
        m_handle.promise().set_continuation(parent_handle);
        return m_handle;
      }

    private:
      std::coroutine_handle<promise_type> m_handle;
    };

    return awaitable{m_handle};
  }

  bool done() const noexcept { return m_handle.done(); }

private:
  task(std::coroutine_handle<promise_type> handle) : m_handle(handle) {}

  detail::unique_coroutine_handle<promise_type> m_handle;
};

} // namespace exec
