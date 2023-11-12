#ifndef HOOD_PROXY_TLS_DEFINITION_H_
#define HOOD_PROXY_TLS_DEFINITION_H_
#include <array>
#include <iostream>
#include <string>
#include <variant>
#include <vector>

#include "tls_definition_raw.hpp"

namespace hood_proxy {
namespace tls {

using SessionID = std::vector<uint8_t>;
using CipherSuite = protocol::CipherSuite;
using CipherSuites = std::vector<CipherSuite>;
using CompressionMethods =
    std::vector<typeof(protocol::CompressionMethods::data[0])>;
namespace extension {

struct ServerName {
  std::string host_name;
};

using NamedGroup = protocol::extension::NamedGroup;
using NamedGroupList = std::vector<NamedGroup>;

struct Extension {
  uint16_t type;
  std::variant<std::nullptr_t, std::vector<uint8_t>, ServerName, NamedGroupList>
      content;
};
using Extensions = std::vector<Extension>;

static inline bool FindHostName(std::string& host_name,
                                const Extensions& extensions) {
  for (auto& extension : extensions) {
    if (extension.type != protocol::extension::Type::server_name) {
      continue;
    }
    auto& server_name = std::get<extension::ServerName>(extension.content);
    host_name = server_name.host_name;
    return true;
  }
  return false;
}

}  // namespace extension

namespace handshake {
struct ClientHello {
  uint16_t legacy_version;
  std::array<uint8_t, 32> random;
  SessionID legacy_session_id;
  CipherSuites cipher_suites;
  CompressionMethods legacy_compression_methods;
  extension::Extensions extensions;
};

struct ServerHello {
  uint16_t legacy_version;
  std::array<uint8_t, 32> random;
  SessionID legacy_session_id_echo;
  CipherSuite cipher_suite;
  uint8_t legacy_compression_method;
  extension::Extensions extensions;
};

struct Message {
  uint8_t type;
  std::variant<std::nullptr_t, std::vector<uint8_t>, ClientHello, ServerHello>
      content;
};
}  // namespace handshake

struct Message {
  uint8_t type;  // protocol::ContentType
  uint16_t legacy_record_version;
  std::variant<std::nullptr_t, std::vector<uint8_t>, handshake::Message>
      content;
};

}  // namespace tls
}  // namespace hood_proxy
#endif  // HOOD_PROXY_TLS_DEFINITION_H_