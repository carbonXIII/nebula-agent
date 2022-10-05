#pragma once

#include <array>
#include <bit>
#include <random>
#include <span>
#include <atomic>

namespace util {
  namespace detail {
    // Type acts as a tag to find the correct operator| overload
    template <typename C>
    struct to_helper {
    };

    // This actually does the work
    template <typename Container, std::ranges::range R>
    requires std::convertible_to<std::ranges::range_value_t<R>, typename Container::value_type>
    Container operator|(R&& r, to_helper<Container>) {
      return Container{r.begin(), r.end()};
    }
  }

  constexpr auto change_byte_order(auto x) {
    if constexpr(std::endian::native != std::endian::big) {
      return std::byteswap(x);
    }
    else {
      return x;
    }
  }

  template <typename To>
  To& convert_span(std::span<std::byte> data) {
    return *reinterpret_cast<To*>(data.data());
  }

  std::array<std::byte, 12> generate_iv() {
    // TODO: should use a better generator
    static constexpr auto gen = []() {
      static std::mt19937 rand;
      return (std::byte)rand();
    };

    std::array<std::byte, 12> ret;
    std::generate(ret.begin(), ret.end(), gen);
    return ret;
  }

  template <auto func, typename... Args>
  struct WithTimeout {
    using clock = std::chrono::system_clock;
    using result = typename std::invoke_result<decltype(func), Args...>::type;

    const clock::duration timeout;
    const std::tuple<Args...> args;

    std::optional<result> res;
    clock::time_point t;


    WithTimeout(clock::duration timeout, Args... args)
      : timeout(timeout), args(std::forward<Args>(args)...) {}

    const result& operator()() {
      if(!res.has_value() || clock::now() - t > timeout) {
        res = std::apply(func, args);
      }

      return res.value();
    }
  };

  template <auto func, typename... Args>
  auto with_timeout(std::chrono::system_clock::duration timeout, Args... args) {
    return WithTimeout<func, std::remove_reference_t<Args>...>(timeout, std::forward<Args>(args)...);
  }

  // Couldn't find an concept for container, however a
  // container is a range, but not a view.
  template <std::ranges::range Container> requires (!std::ranges::view<Container>)
  auto to() { return detail::to_helper<Container>{}; }

  template <typename T>
  auto from_string_view(auto s) {
    T ret;
    auto [ptr, ec] = std::from_chars(s.begin(), s.end(), ret);
    if(ec != std::errc()) throw std::runtime_error("FIXME");
    return ret;
  };
}
