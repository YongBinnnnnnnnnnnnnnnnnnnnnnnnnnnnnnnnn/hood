#ifndef HOOD_PROXY_SSL_DEFINITION_RAW_H_
#define HOOD_PROXY_SSL_DEFINITION_RAW_H_
#include <boost/endian/conversion.hpp>
#include <cstdint>

namespace hood_proxy {
namespace ssl {
#pragma pack(push, 1)

struct ContentType {
  static constexpr uint8_t invalid = 0;
  static constexpr uint8_t change_cipher_spec = 20;
  static constexpr uint8_t alert = 21;
  static constexpr uint8_t handshake = 22;
  static constexpr uint8_t application_data = 23;
  static constexpr uint8_t undefined = 255;
};

struct TLSPlaintext {
  uint8_t type;
  uint16_t legacy_record_version;
  uint16_t mesage_length;
  uint8_t data[];
};

struct HandshakeType {
  static constexpr uint8_t client_hello = 1;
  static constexpr uint8_t server_hello = 2;
  static constexpr uint8_t new_session_ticket = 4;
  static constexpr uint8_t end_of_early_data = 5;
  static constexpr uint8_t encrypted_extensions = 8;
  static constexpr uint8_t certificate = 11;
  static constexpr uint8_t certificate_request = 13;
  static constexpr uint8_t certificate_verify = 15;
  static constexpr uint8_t finished = 20;
  static constexpr uint8_t key_update = 24;
  static constexpr uint8_t message_hash = 254;
  static constexpr uint8_t undefined = 255;
};

struct uint24_t {
  uint8_t high;
  uint8_t middle;
  uint8_t low;
};

static constexpr uint32_t GetUint24Value(uint24_t data) {
  return uint32_t((uint32_t(data.high) << 16) | (uint32_t(data.middle) << 8) |
                  uint32_t(data.middle));
}
static constexpr void SetUint24Value(uint24_t& this_, uint32_t value) {
  this_.high = value >> 16;
  this_.middle = value >> 8;
  this_.low = value;
}
struct RawHandshake {
  uint8_t msg_type; /* handshake type */
  uint24_t length;  /* remaining bytes in message */
  uint8_t data[];
};
struct RawSessionID {
  uint8_t length;
  uint8_t value[];
};

using CipherSuite = uint16_t;

struct CipherSuites {
  uint16_t length;
  CipherSuite data[];
};

struct CompressionMethods {
  uint16_t length;
  uint8_t data[];
};

struct Extension {
  uint16_t type;
  uint16_t length;
  uint8_t data[];
};
struct Extensions {
  uint16_t length;
  Extension data[];
};

struct RawClientHello {
  uint16_t legacy_version;
  uint8_t random[32];
  RawSessionID legacy_session_id;
  CipherSuites cipher_suites;
  CompressionMethods legacy_compression_methods;
  Extensions extensions;
};

struct RawServerHello {
  uint16_t legacy_version;
  uint8_t random[32];
  RawSessionID legacy_session_id_echo;
  CipherSuite cipher_suite;
  uint8_t legacy_compression_method;
  Extensions extensions;
};

#pragma pack(pop)
}  // namespace ssl
}  // namespace hood_proxy
#endif  // HOOD_PROXY_SSL_DEFINITION_RAW_H_