#ifndef HOOD_PROXY_TLS_SECURITY_POLICY_H_
#define HOOD_PROXY_TLS_SECURITY_POLICY_H_

#include <cassert>
#include <chrono>
#include <memory>
#include <set>

#include "configuration.hpp"
#include "engine.hpp"
#include "tls_definition.hpp"

namespace hood_proxy {
namespace tls {

class SecurityPolicy {
 public:
  static constexpr VersionType lowest_allowed_version =
      protocol::Version::TLS_1_2;  // RFC9325
  static const std::set<CipherSuite> allowed_cipher_suites;

  static constexpr inline bool CheckCipherSuites(
      const CipherSuites& cipher_suites) {
    for (auto& cipher_suite : cipher_suites) {
      if (SecurityPolicy::allowed_cipher_suites.contains(cipher_suite)) {
        return true;
      }
    }
    return false;
  }

  static constexpr inline bool HardenVersions(handshake::Message& handshake) {
    if (handshake.type == protocol::handshake::Type::client_hello) {
      auto message = std::get<handshake::ClientHello>(handshake.content);
      HardenVersionsInternal(message);
    } else if (handshake.type == protocol::handshake::Type::server_hello) {
      auto message = std::get<handshake::ServerHello>(handshake.content);
      HardenVersionsInternal(message);
    }
    return true;
  }

 private:
  template <typename T>
  static constexpr inline bool HardenVersionsInternal(T& message) {
    bool support_allowed_version =
        message.legacy_version >= lowest_allowed_version;
    if (!support_allowed_version) {
      message.legacy_version = lowest_allowed_version;
    }
    for (auto& extension : message.extensions) {
      if (extension.type != protocol::extension::Type::supported_versions) {
        continue;
      }
      auto& versions =
          std::get<extension::SupportedVersions>(extension.content);
      versions.erase(
          std::remove_if(versions.begin(), versions.end(),
                         [](VersionType version) {
                           return version <
                                  SecurityPolicy::lowest_allowed_version;
                         }),
          versions.end());
      support_allowed_version = support_allowed_version || versions.size();
    }
    return support_allowed_version;
  }
};

}  // namespace tls
}  // namespace hood_proxy
#endif  // HOOD_PROXY_TLS_SECURITY_POLICY_H_