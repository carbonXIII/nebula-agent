#pragma once

#include <array>
#include <bit>
#include <random>
#include <span>
#include <atomic>

namespace util {
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
}
