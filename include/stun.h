#pragma once

#include <algorithm>
#include <array>
#include <span>
#include <type_traits>
#include <variant>
#include <optional>
#include <chrono>

#include "util.h"

#include <fmt/core.h>
#include <boost/asio.hpp>

namespace stun {
  namespace detail {
    constexpr auto COOKIE = util::change_byte_order((uint32_t)0x2112A442);

    enum message_type : uint16_t {
      BINDING_REQUEST = util::change_byte_order((uint16_t)0x1),
      BINDING_RESPONSE = util::change_byte_order((uint16_t)0x101),
    };

    enum attribute_type : uint16_t {
      XOR_MAPPED_ADDRESS = util::change_byte_order((uint16_t)0x20)
    };

#pragma pack(push, 1)
    template <uint16_t max_length = 0>
    struct message {
      uint16_t type;
      uint16_t length;
      uint32_t cookie;
      std::array<std::byte, 12> transaction_id; // iv

      using attributes_t = std::conditional_t<max_length == 0,
                                              std::monostate,
                                              std::array<std::byte, max_length>>;
      [[no_unique_address]] attributes_t attributes_;

      message() {}

      message(uint16_t type)
        : type(type),
          length(util::change_byte_order(max_length)),
          cookie(COOKIE),
          transaction_id(util::generate_iv()) {}

      auto as_buffer() {
        return boost::asio::buffer(static_cast<void*>(this), sizeof(*this));
      }

      void for_attributes(auto func) {
        if constexpr (max_length != 0) {
          std::span attr(attributes_);
          for(size_t i = 0; i < length;) {
            auto [type, length] = util::convert_span<std::array<uint16_t, 2>>(attr.subspan(i, 4));
            length = util::change_byte_order(length);

            auto data = attr.subspan(i + 4, length);
            func(type, length, data);

            i += length;
            break;
          }
        }
      }
    };
#pragma pack(pop)
  }

  struct IpAndPort {
    std::string ip;
    int port;
  };

  IpAndPort stun(int local_port) {
    using boost::asio::ip::udp;

    boost::asio::io_context io_context;
    udp::resolver resolver(io_context);
    auto results = resolver.resolve("stun.l.google.com", "19302");
    auto remote_endpoint = results.begin()->endpoint();

    udp::socket socket(io_context, udp::endpoint(udp::v4(), local_port));

    detail::message<0> req(detail::BINDING_REQUEST);
    socket.send_to(req.as_buffer(), remote_endpoint);

    detail::message<1024> resp;
    socket.receive(resp.as_buffer());

    std::string ip;
    int port;
    resp.for_attributes([&](uint16_t type, uint16_t length, std::span<std::byte> data) {
      if(type == detail::XOR_MAPPED_ADDRESS) {
        // TODO: assert length
        port = util::change_byte_order(util::convert_span<uint16_t>(data.subspan(2, 2))) ^ 0x2112;
        ip = fmt::format("{}.{}.{}.{}",
                         (uint8_t)data[4] ^ 0x21,
                         (uint8_t)data[5] ^ 0x12,
                         (uint8_t)data[6] ^ 0xA4,
                         (uint8_t)data[7] ^ 0x42);
      }
    });

    return { ip, port };
  }
}
