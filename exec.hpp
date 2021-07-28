#include <coroutine>
#include <exception>
#include <memory>

namespace exec {

namespace detail {

template <typename Promise>
class unique_coroutine_handle {
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

template <typename T>
class return_value_promise {
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

template <typename T>
class value_promise : public return_value_promise<T> {
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

template <typename T>
struct base_promise {
  using type = value_promise<T>;
};

template <typename T>
struct base_promise<T &> {
  using type = reference_promise<T>;
};

template <>
struct base_promise<void> {
  using type = void_promise;
};

template <typename T>
using base_promise_t = base_promise<T>::type;

template <typename T>
class awaitable_container {
 public:
  awaitable_container(T aw) : m_original(std::forward<T>(aw)) {}
  auto &get() { return m_original; }

 private:
  T m_original;
};

template <typename T>
struct get_base_awaitable_impl {};

template <member_co_awaitable T>
struct get_base_awaitable_impl<T> {
  using Type = decltype(std::declval<T>().operator co_await());
};

template <non_member_co_awaitable T>
struct get_base_awaitable_impl<T> {
  using Type = decltype(operator co_await(std::declval<T>()));
};

template <typename T>
using get_base_awaitable_t = get_base_awaitable_impl<T>::Type;

template <typename T>
concept member_co_awaitable = requires(T a) {
  a.operator co_await();
};

template <typename T>
concept non_member_co_awaitable = requires(T a) {
  operator co_await(a);
};

template <typename T>
concept generated_co_awaitable =
    member_co_awaitable<T> || non_member_co_awaitable<T>;

template <generated_co_awaitable T>
class awaitable_container<T> {
 public:
  awaitable_container(T aw)
      : m_original(std::forward<T>(aw)),
        m_base(get_base_awaitable(m_original)) {}
  auto &get() { return m_base; }

 private:
  T m_original;
  get_base_awaitable_t<T> m_base;
};

template <typename T>
using non_void_awaited_t =
    std::conditional_t<std::is_void_v<awaited_t<T>>, void_t, awaited_t<T>>;

template <typename... T>
class gather_impl {
 public:
  gather_impl(T... args)
      : m_await(detail::awaitable_container<T>(std::forward<T>(args))...) {}

  bool await_ready() noexcept {
    std::size_t n_ready = 0;
    constexpr_for<0UL, std::tuple_size_v<decltype(m_await)>, 1UL>(
        [this, n_ready](auto i) mutable {
          n_ready += std::get<i.value>(m_await).get().await_ready();
        });

    return n_ready == std::tuple_size_v<decltype(m_await)>;
  }
  void await_suspend(std::coroutine_handle<> handle) noexcept {
    std::coroutine_handle<> inc_handle =
        [this](std::coroutine_handle<> resume) -> detail::task_executor {
      for (auto i = 0UL; i < std::tuple_size_v<decltype(m_await)>; ++i) {
        co_await std::suspend_always{};
      }
      resume.resume();
    }(handle)
                                                      .m_handle;

    constexpr_for<0UL, std::tuple_size_v<decltype(m_await)>, 1UL>(
        [this, inc_handle](auto i) {
          using S = decltype(std::get<i.value>(m_await).get().await_suspend(
              inc_handle));
          if constexpr (std::is_void_v<S>) {
            std::get<i.value>(m_await).get().await_suspend(inc_handle);
          } else if constexpr (std::is_same_v<S, bool>) {
            if (!std::get<i.value>(m_await).get().await_suspend(inc_handle)) {
              inc_handle.resume();
            }
          } else {
            std::get<i.value>(m_await).get().await_suspend(inc_handle).resume();
          }
        });
  }
  auto await_resume() noexcept {
    return std::apply(
        [](auto &&...args) -> std::tuple<detail::non_void_awaited_t<T>...> {
          return {non_void_resume(args.get())...};
        },
        m_await);
  }

 protected:
  template <typename U>
  static decltype(auto) non_void_resume(U &await) {
    if constexpr (std::is_void_v<decltype(await.await_resume())>) {
      await.await_resume();
      return void_t{};
    } else {
      return await.await_resume();
    }
  }

  template <auto Start, auto End, auto Inc, class F>
  static constexpr void constexpr_for(F &&f) {
    if constexpr (Start < End) {
      f(std::integral_constant<decltype(Start), Start>());
      constexpr_for<Start + Inc, End, Inc>(f);
    }
  }

  std::tuple<detail::awaitable_container<T>...> m_await;
};

template <typename T>
using base_awaitable_t =
    decltype(base_awaitable(std::declval<std::remove_reference_t<T>>()));

template <typename T>
using awaited_t = decltype(std::declval<base_awaitable_t<T>>().await_resume());

template <typename T>
concept await_suspend_returnable = std::same_as<T, bool> ||
    std::same_as<T, void> || std::same_as<T, std::coroutine_handle<>>;

template <typename T>
class task_executor_return {
 public:
  class promise_type final : public base_promise_t<T> {
   public:
    task_executor_return<T> get_return_object() noexcept {
      return task_executor_return(
          std::coroutine_handle<promise_type>::from_promise(*this));
    }
    std::suspend_never initial_suspend() const noexcept { return {}; }
    std::suspend_always final_suspend() const noexcept { return {}; }
  };

  std::coroutine_handle<promise_type> m_handle;
};

}  // namespace detail

template <typename T>
class task {
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
        std::coroutine_handle<> await_suspend(
            std::coroutine_handle<promise_type> handle) noexcept {
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

      std::coroutine_handle<> await_suspend(
          std::coroutine_handle<> parent_handle) noexcept {
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

class sleep_for {
 public:
  sleep_for(double d)
      : duration(d)
  // m_fd(timerfd_create(CLOCK_MONOTONIC, 0)), m_time(static_cast<timespec>(d))
  {}
  bool await_ready() const noexcept {
    // return m_time.tv_sec < 0 || (m_time.tv_sec == 0 && m_time.tv_nsec == 0);
    return true;
  }
  void await_resume() const noexcept {}
  void await_suspend(std::coroutine_handle<> handle) noexcept {
    // itimerspec tspec{.it_value=m_time};
    // timerfd_settime(m_fd.get(), 0, &tspec, nullptr);
    // event_loop::add_reader(m_fd.get(), handle);
  }
  void await_cancel() noexcept {
    // event_loop::remove_fd(m_fd.get());
  }

 private:
  // detail::unique_fd m_fd;
  // timespec m_time;
  double duration;
};

template <typename T>
concept awaitable = requires(detail::base_awaitable_t<T> a) {
  { a.await_ready() } -> std::same_as<bool>;
  {
    a.await_suspend(std::coroutine_handle<>{})
    } -> detail::await_suspend_returnable<>;
  {a.await_resume()};
};

template <awaitable... T>
auto gather(T &&...args) {
  return detail::gather_impl<T...>(std::forward<T>(args)...);
}

class event_loop {
 public:
  template <typename T>
  static T run(task<T> root_task) {
    auto exec = [](task<T> &t) -> detail::task_executor_return<T> {
      co_return co_await t;
    }(root_task);

    // io_loop(root_task);

    if constexpr (std::is_void_v<T>) {
      exec.m_handle.destroy();
    } else {
      T result = exec.m_handle.promise().result();
      exec.m_handle.destroy();
      return result;
    }
  }
};

}  // namespace exec
