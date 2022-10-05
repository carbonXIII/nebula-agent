#pragma once

#include "stun.h"
#include "util.h"

#include <fmt/core.h>
#include <crow.h>

namespace server {
  struct App: crow::SimpleApp {
    App(int port)
      : cache(util::with_timeout<stun::stun>(std::chrono::minutes(15), port)) {
      route<crow::black_magic::get_parameter_tag("/hosts")>("/hosts") ([this]() {
        return get_hosts();
      });
    }

    crow::json::wvalue get_hosts() {
      auto [ip, port] = cache();
      std::string address = fmt::format("{}:{}", ip, port);

      return crow::json::wvalue::object {
        {"static_host_map", crow::json::wvalue::object {
          {"192.168.100.1", crow::json::wvalue::list({address})}
        }}
      };
    }

    void start() {
      thread.emplace([this](){ run(); });
    }

    std::optional<std::jthread> thread;
    util::WithTimeout<stun::stun, int> cache;
  };
}
