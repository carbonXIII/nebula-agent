#pragma once

#include <string_view>

#include <cpr/cpr.h>

namespace client {
  namespace yaml = YAML;

  // TODO: authentication
  auto fetch_config(const std::string& endpoint) {
    auto resp = cpr::Get(cpr::Url{endpoint});
    if(resp.status_code != 200)
      throw std::runtime_error("FIXME");
    return yaml::Load(resp.text);
  }
}
