#pragma once

#include "stun.h"
#include "util.h"

#include <fmt/core.h>
#include <crow.h>

struct Server: crow::SimpleApp {
  Server(int port)
    : cache(util::with_timeout<stun::stun>(std::chrono::minutes(15), port)) {
    route<crow::black_magic::get_parameter_tag("/hosts")>("/hosts") ([this]() {
      return get_hosts();
    });
  }

  crow::json::wvalue get_hosts() {
    auto [ip, port] = cache();
    std::string address = fmt::format("{}:{}", ip, port);

    return crow::json::wvalue::object {
      {"192.168.100.1", crow::json::wvalue::list({address})}
    };
  }

  util::WithTimeout<stun::stun, int> cache;
};
