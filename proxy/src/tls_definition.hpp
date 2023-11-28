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
using VersionType = protocol::VersionType;
using CompressionMethods =
    std::vector<typeof(protocol::CompressionMethods::data[0])>;
namespace extension {

struct ServerName {
  std::string host_name;
};

using NamedGroup = protocol::extension::NamedGroup;
using NamedGroupList = std::vector<NamedGroup>;
using SupportedVersions = std::vector<VersionType>;

struct Extension {
  uint16_t type;
  std::variant<std::nullptr_t, std::vector<uint8_t>, std::vector<uint16_t>,
               ServerName>
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

struct HelloRetryType {
  static constexpr const uintptr_t NOT_RETRY = 0;
  static constexpr const uintptr_t FOR_TLS_1_1 = 1;
  static constexpr const uintptr_t FOR_TLS_1_2 = 2;
  static constexpr const uintptr_t FOR_TLS_1_3 = 3;
  static constexpr const uintptr_t FOR_UNKNOWN = 4;
};

struct ServerHello {
  uint16_t legacy_version;
  std::array<uint8_t, 32> random;
  uintptr_t hello_retry_type;
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

using Messages = std::vector<Message>;

}  // namespace handshake

struct Message {
  uint8_t type;  // protocol::ContentType
  uint16_t legacy_record_version;
  std::variant<std::nullptr_t, std::vector<uint8_t>, handshake::Messages>
      content;
};

}  // namespace tls
}  // namespace hood_proxy
#endif  // HOOD_PROXY_TLS_DEFINITION_H_