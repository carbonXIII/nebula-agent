#include <array>
#include <random>
#include <string_view>
#include <ranges>
#include <vector>
#include <charconv>
#include <sstream>

#include <fmt/core.h>
#include <yaml-cpp/yaml.h>

#include <server.h>
#include <client.h>

namespace yaml = YAML;

std::array<long, 2> parse_port_range(const std::string_view s) {
  static constexpr std::string_view delim = "-";
  auto ports = std::ranges::split_view(s, delim)
    | std::views::transform([](auto s) { return util::from_string_view<long>(s); })
    | util::to<std::vector<long>>();

  if(ports.size() == 1) {
    return {ports[0], ports[0]};
  } else if(ports.size() == 2) {
    return {ports[0], ports[1]};
  } else {
    throw std::runtime_error("FIXME");
  }
}

int main() {
  auto config = yaml::LoadFile("config.yml");
  auto port_range = parse_port_range(config["listen"]["port"].as<std::string>());

  std::random_device gen;
  std::uniform_int_distribution distr(port_range[0], port_range[1]);
  int port = distr(gen);
  fmt::print("Choosing port: {}\n", port); // FIXME: better logs

  bool is_lighthouse = config["lighthouse"]["am_lighthouse"].as<bool>();

  std::optional<server::App> app;
  if(is_lighthouse) {
    app.emplace(port);
    app->port(8080);
    app->start(); // FIXME: load port from agent config
  }

  auto reload_config = [&]() {
    config["listen"]["port"] = port;

    if(!is_lighthouse) {
      auto url = config["agent"]["lighthouse"]["url"].as<std::string>();
      std::string lighthouse_endpoint = fmt::format("{}/hosts", url);
      auto extra_config = client::fetch_config(lighthouse_endpoint);
      for(const auto& entry: extra_config) {
        config[entry.first] = entry.second;
      }
    }

    std::ostringstream ss; ss << config;
    fmt::print("new config:\n{}\n", ss.view());
  };

  reload_config();

  // run nebula child

  // on SIGHUP or several hour timeout, re-run config building step
  // on any config change, SIGHUP child

  return 0;
}
