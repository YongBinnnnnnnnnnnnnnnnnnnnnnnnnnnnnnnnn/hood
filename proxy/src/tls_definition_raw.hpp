#ifndef HOOD_PROXY_TLS_DEFINITION_RAW_H_
#define HOOD_PROXY_TLS_DEFINITION_RAW_H_
#include <boost/endian/conversion.hpp>
#include <cstdint>

namespace hood_proxy {
namespace tls {
namespace protocol {
#pragma pack(push, 1)

struct Version {
  static constexpr uint16_t TLS_1_0 = 0x301;
  static constexpr uint16_t TLS_1_1 = 0x302;
  static constexpr uint16_t TLS_1_2 = 0x303;
  static constexpr uint16_t TLS_1_3 = 0x304;
};

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
  uint16_t message_length;
  uint8_t data[];
};

static constexpr auto MAX_MESSAGE_SIZE = sizeof(TLSPlaintext) + UINT16_MAX;

struct uint24_t {
  uint8_t high;
  uint8_t middle;
  uint8_t low;
};

static constexpr uint32_t GetUint24Value(uint24_t data) {
  return uint32_t((uint32_t(data.high) << 16) | (uint32_t(data.middle) << 8) |
                  uint32_t(data.low));
}
static constexpr void SetUint24Value(uint24_t& this_, uint32_t value) {
  this_.high = value >> 16;
  this_.middle = value >> 8;
  this_.low = value;
}
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

namespace extension {
struct Type {
  static constexpr uint16_t server_name = 0;             //[RFC6066][RFC9261]
  static constexpr uint16_t max_fragment_length = 1;     //[RFC6066][RFC8449]
  static constexpr uint16_t client_certificate_url = 2;  //[RFC6066]
  static constexpr uint16_t trusted_ca_keys = 3;         //[RFC6066]
  static constexpr uint16_t truncated_hmac =
      4;  //[RFC6066][IESG Action 2018-08-16]
  static constexpr uint16_t status_request = 5;  //[RFC6066]
  static constexpr uint16_t user_mapping = 6;    //[RFC4681]
  static constexpr uint16_t client_authz = 7;    //[RFC5878]
  static constexpr uint16_t server_authz = 8;    //[RFC5878]
  static constexpr uint16_t cert_type = 9;       //[RFC6091]
  static constexpr uint16_t supported_groups = 10;
  //(renamed from "elliptic_curves") [RFC8422][RFC7919]
  static constexpr uint16_t ec_point_formats = 11;      //[RFC8422]
  static constexpr uint16_t srp = 12;                   //[RFC5054]
  static constexpr uint16_t signature_algorithms = 13;  //[RFC8446]
  static constexpr uint16_t use_srtp = 14;              //[RFC5764]
  static constexpr uint16_t heartbeat = 15;             //[RFC6520]
  static constexpr uint16_t application_layer_protocol_negotiation =
      16;                                                       //[RFC7301]
  static constexpr uint16_t status_request_v2 = 17;             //[RFC6961]
  static constexpr uint16_t signed_certificate_timestamp = 18;  //[RFC6962]
  static constexpr uint16_t client_certificate_type = 19;       //[RFC7250]
  static constexpr uint16_t server_certificate_type = 20;       //[RFC7250]
  static constexpr uint16_t padding = 21;                       //[RFC7685]
  static constexpr uint16_t encrypt_then_mac = 22;              //[RFC7366]
  static constexpr uint16_t extended_master_secret = 23;        //[RFC7627]
  static constexpr uint16_t token_binding = 24;                 //[RFC8472]
  static constexpr uint16_t cached_info = 25;                   //[RFC7924]
  static constexpr uint16_t tls_lts = 26;  //[draft-gutmann-tls-lts]
  static constexpr uint16_t compress_certificate = 27;      //[RFC8879]
  static constexpr uint16_t record_size_limit = 28;         //[RFC8449]
  static constexpr uint16_t pwd_protect = 29;               //[RFC8492]
  static constexpr uint16_t pwd_clear = 30;                 //[RFC8492]
  static constexpr uint16_t password_salt = 31;             //[RFC8492]
  static constexpr uint16_t ticket_pinning = 32;            //[RFC8672]
  static constexpr uint16_t tls_cert_with_extern_psk = 33;  //[RFC8773]
  static constexpr uint16_t delegated_credential = 34;      //[RFC9345]
  static constexpr uint16_t session_ticket =
      35;  // (renamed from "SessionTicket TLS") [RFC5077][RFC8447]
  static constexpr uint16_t TLMSP = 36;                  //[ETSI TS 103 523-2]
  static constexpr uint16_t TLMSP_proxying = 37;         //[ETSI TS 103 523-2]
  static constexpr uint16_t TLMSP_delegate = 38;         //[ETSI TS 103 523-2]
  static constexpr uint16_t supported_ekt_ciphers = 39;  //[RFC8870]
  // static constexpr uint16_t Reserved = 40;  //[tls-reg-review mailing list]
  static constexpr uint16_t pre_shared_key = 41;          //[RFC8446]
  static constexpr uint16_t early_data = 42;              //[RFC8446]
  static constexpr uint16_t supported_versions = 43;      //[RFC8446]
  static constexpr uint16_t cookie = 44;                  //[RFC8446]
  static constexpr uint16_t psk_key_exchange_modes = 45;  //[RFC8446]
  // static constexpr uint16_t Reserved = 46;  //[tls-reg-review mailing list]
  static constexpr uint16_t certificate_authorities = 47;    //[RFC8446]
  static constexpr uint16_t oid_filters = 48;                //[RFC8446]
  static constexpr uint16_t post_handshake_auth = 49;        //[RFC8446]
  static constexpr uint16_t signature_algorithms_cert = 50;  //[RFC8446]
  static constexpr uint16_t key_share = 51;                  //[RFC8446]
  static constexpr uint16_t transparency_info = 52;          //[RFC9162]
  // static constexpr uint16_t connection_id = 53; // (deprecated) [RFC9146]
  static constexpr uint16_t connection_id = 54;              //[RFC9146]
  static constexpr uint16_t external_id_hash = 55;           //[RFC8844]
  static constexpr uint16_t external_session_id = 56;        //[RFC8844]
  static constexpr uint16_t quic_transport_parameters = 57;  //[RFC9001]
  static constexpr uint16_t ticket_request = 58;             //[RFC9149]
  static constexpr uint16_t dnssec_chain = 59;  //[RFC9102][RFC Errata 6860]
  static constexpr uint16_t sequence_number_encryption_algorithms =
      60;  //[draft-pismenny-tls-dtls-plaintext-sequence-number-01]
};

struct ServerNameType {
  static constexpr uint8_t host_name = 0;
  static constexpr uint8_t undefined = 255;
};

struct ServerName {
  uint8_t name_type;
  uint16_t length;
  char name[];
};

struct ServerNameList {
  uint16_t length;
  ServerName server_name_list[];
};

struct NamedGroupValue {
  // RFC 8446 4.2.7
  /* Elliptic Curve Groups (ECDHE) */
  static constexpr uint16_t secp256r1 = 0x0017;
  static constexpr uint16_t secp384r1 = 0x0018;
  static constexpr uint16_t secp521r1 = 0x0019;
  static constexpr uint16_t x25519 = 0x001D;
  static constexpr uint16_t x448 = 0x001E;

  /* Finite Field Groups (DHE) */
  static constexpr uint16_t ffdhe2048 = 0x0100;
  static constexpr uint16_t ffdhe3072 = 0x0101;
  static constexpr uint16_t ffdhe4096 = 0x0102;
  static constexpr uint16_t ffdhe6144 = 0x0103;
  static constexpr uint16_t ffdhe8192 = 0x0104;

  /* Reserved Code Points */
  // ffdhe_private_use(0x01FC..0x01FF),
  // ecdhe_private_use(0xFE00..0xFEFF),
  static constexpr uint16_t undefined = 0xFFFF;
};

using NamedGroup = uint16_t;

struct NamedGroupList {
  uint16_t vector_size;
  uint16_t length;
  NamedGroup groups[];
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

}  // namespace extension

namespace handshake {

struct Type {
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

struct RawHandshake {
  uint8_t msg_type; /* handshake type */
  uint24_t length;  /* remaining bytes in message */
  uint8_t data[];
};

struct RawClientHello {
  struct FixedLengthHead {
    uint16_t legacy_version;
    uint8_t random[32];
  } fixed_length_head;
  RawSessionID legacy_session_id;
  // CipherSuites cipher_suites;
  //  CompressionMethods legacy_compression_methods;
  //  extension::Extensions extensions;
};

struct RawServerHello {
  struct FixedLengthHead {
    uint16_t legacy_version;
    uint8_t random[32];
  } fixed_length_head;
  RawSessionID legacy_session_id_echo;
  // CipherSuite cipher_suite;
  // uint8_t legacy_compression_method;
  // extension::Extensions extensions;
};
}  // namespace handshake

#pragma pack(pop)
}  // namespace protocol
}  // namespace tls
}  // namespace hood_proxy
#endif  // HOOD_PROXY_TLS_DEFINITION_RAW_H_