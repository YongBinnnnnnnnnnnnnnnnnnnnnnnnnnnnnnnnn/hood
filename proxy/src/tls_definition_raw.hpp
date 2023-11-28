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
using VersionType = uint16_t;

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

struct CipherSuiteValue {
  static constexpr uint16_t TLS_NULL_WITH_NULL_NULL = 0x0000;  // [RFC5246]
  static constexpr uint16_t TLS_RSA_WITH_NULL_MD5 = 0x0001;    // [RFC5246]
  static constexpr uint16_t TLS_RSA_WITH_NULL_SHA = 0x0002;    // [RFC5246]
  static constexpr uint16_t TLS_RSA_EXPORT_WITH_RC4_40_MD5 =
      0x0003;  // [RFC4346][RFC6347]
  static constexpr uint16_t TLS_RSA_WITH_RC4_128_MD5 =
      0x0004;  // [RFC5246][RFC6347]
  static constexpr uint16_t TLS_RSA_WITH_RC4_128_SHA =
      0x0005;  // [RFC5246][RFC6347]
  static constexpr uint16_t TLS_RSA_EXPORT_WITH_RC2_CBC_40_MD5 =
      0x0006;                                                    // [RFC4346]
  static constexpr uint16_t TLS_RSA_WITH_IDEA_CBC_SHA = 0x0007;  // [RFC8996]
  static constexpr uint16_t TLS_RSA_EXPORT_WITH_DES40_CBC_SHA =
      0x0008;                                                   // [RFC4346]
  static constexpr uint16_t TLS_RSA_WITH_DES_CBC_SHA = 0x0009;  // [RFC8996]
  static constexpr uint16_t TLS_RSA_WITH_3DES_EDE_CBC_SHA =
      0x000A;  // [RFC5246]
  static constexpr uint16_t TLS_DH_DSS_EXPORT_WITH_DES40_CBC_SHA =
      0x000B;                                                      // [RFC4346]
  static constexpr uint16_t TLS_DH_DSS_WITH_DES_CBC_SHA = 0x000C;  // [RFC8996]
  static constexpr uint16_t TLS_DH_DSS_WITH_3DES_EDE_CBC_SHA =
      0x000D;  // [RFC5246]
  static constexpr uint16_t TLS_DH_RSA_EXPORT_WITH_DES40_CBC_SHA =
      0x000E;                                                      // [RFC4346]
  static constexpr uint16_t TLS_DH_RSA_WITH_DES_CBC_SHA = 0x000F;  // [RFC8996]
  static constexpr uint16_t TLS_DH_RSA_WITH_3DES_EDE_CBC_SHA =
      0x0010;  // [RFC5246]
  static constexpr uint16_t TLS_DHE_DSS_EXPORT_WITH_DES40_CBC_SHA =
      0x0011;                                                       // [RFC4346]
  static constexpr uint16_t TLS_DHE_DSS_WITH_DES_CBC_SHA = 0x0012;  // [RFC8996]
  static constexpr uint16_t TLS_DHE_DSS_WITH_3DES_EDE_CBC_SHA =
      0x0013;  // [RFC5246]
  static constexpr uint16_t TLS_DHE_RSA_EXPORT_WITH_DES40_CBC_SHA =
      0x0014;                                                       // [RFC4346]
  static constexpr uint16_t TLS_DHE_RSA_WITH_DES_CBC_SHA = 0x0015;  // [RFC8996]
  static constexpr uint16_t TLS_DHE_RSA_WITH_3DES_EDE_CBC_SHA =
      0x0016;  // [RFC5246]
  static constexpr uint16_t TLS_DH_anon_EXPORT_WITH_RC4_40_MD5 =
      0x0017;  // [RFC4346][RFC6347]
  static constexpr uint16_t TLS_DH_anon_WITH_RC4_128_MD5 =
      0x0018;  // [RFC5246][RFC6347]
  static constexpr uint16_t TLS_DH_anon_EXPORT_WITH_DES40_CBC_SHA =
      0x0019;                                                       // [RFC4346]
  static constexpr uint16_t TLS_DH_anon_WITH_DES_CBC_SHA = 0x001A;  // [RFC8996]
  static constexpr uint16_t TLS_DH_anon_WITH_3DES_EDE_CBC_SHA =
      0x001B;  // [RFC5246]
  // 0x001C-1D 	Reserved to avoid conflicts with SSLv3 [RFC5246]
  static constexpr uint16_t TLS_KRB5_WITH_DES_CBC_SHA = 0x001E;  // [RFC2712]
  static constexpr uint16_t TLS_KRB5_WITH_3DES_EDE_CBC_SHA =
      0x001F;  // [RFC2712]
  static constexpr uint16_t TLS_KRB5_WITH_RC4_128_SHA =
      0x0020;  // [RFC2712][RFC6347]
  static constexpr uint16_t TLS_KRB5_WITH_IDEA_CBC_SHA = 0x0021;  // [RFC2712]
  static constexpr uint16_t TLS_KRB5_WITH_DES_CBC_MD5 = 0x0022;   // [RFC2712]
  static constexpr uint16_t TLS_KRB5_WITH_3DES_EDE_CBC_MD5 =
      0x0023;  // [RFC2712]
  static constexpr uint16_t TLS_KRB5_WITH_RC4_128_MD5 =
      0x0024;  // [RFC2712][RFC6347]
  static constexpr uint16_t TLS_KRB5_WITH_IDEA_CBC_MD5 = 0x0025;  // [RFC2712]
  static constexpr uint16_t TLS_KRB5_EXPORT_WITH_DES_CBC_40_SHA =
      0x0026;  // [RFC2712]
  static constexpr uint16_t TLS_KRB5_EXPORT_WITH_RC2_CBC_40_SHA =
      0x0027;  // [RFC2712]
  static constexpr uint16_t TLS_KRB5_EXPORT_WITH_RC4_40_SHA =
      0x0028;  // [RFC2712][RFC6347]
  static constexpr uint16_t TLS_KRB5_EXPORT_WITH_DES_CBC_40_MD5 =
      0x0029;  // [RFC2712]
  static constexpr uint16_t TLS_KRB5_EXPORT_WITH_RC2_CBC_40_MD5 =
      0x002A;  // [RFC2712]
  static constexpr uint16_t TLS_KRB5_EXPORT_WITH_RC4_40_MD5 =
      0x002B;  // [RFC2712][RFC6347]
  static constexpr uint16_t TLS_PSK_WITH_NULL_SHA = 0x002C;         // [RFC4785]
  static constexpr uint16_t TLS_DHE_PSK_WITH_NULL_SHA = 0x002D;     // [RFC4785]
  static constexpr uint16_t TLS_RSA_PSK_WITH_NULL_SHA = 0x002E;     // [RFC4785]
  static constexpr uint16_t TLS_RSA_WITH_AES_128_CBC_SHA = 0x002F;  // [RFC5246]
  static constexpr uint16_t TLS_DH_DSS_WITH_AES_128_CBC_SHA =
      0x0030;  // [RFC5246]
  static constexpr uint16_t TLS_DH_RSA_WITH_AES_128_CBC_SHA =
      0x0031;  // [RFC5246]
  static constexpr uint16_t TLS_DHE_DSS_WITH_AES_128_CBC_SHA =
      0x0032;  // [RFC5246]
  static constexpr uint16_t TLS_DHE_RSA_WITH_AES_128_CBC_SHA =
      0x0033;  // [RFC5246]
  static constexpr uint16_t TLS_DH_anon_WITH_AES_128_CBC_SHA =
      0x0034;                                                       // [RFC5246]
  static constexpr uint16_t TLS_RSA_WITH_AES_256_CBC_SHA = 0x0035;  // [RFC5246]
  static constexpr uint16_t TLS_DH_DSS_WITH_AES_256_CBC_SHA =
      0x0036;  // [RFC5246]
  static constexpr uint16_t TLS_DH_RSA_WITH_AES_256_CBC_SHA =
      0x0037;  // [RFC5246]
  static constexpr uint16_t TLS_DHE_DSS_WITH_AES_256_CBC_SHA =
      0x0038;  // [RFC5246]
  static constexpr uint16_t TLS_DHE_RSA_WITH_AES_256_CBC_SHA =
      0x0039;  // [RFC5246]
  static constexpr uint16_t TLS_DH_anon_WITH_AES_256_CBC_SHA =
      0x003A;                                                   // [RFC5246]
  static constexpr uint16_t TLS_RSA_WITH_NULL_SHA256 = 0x003B;  // [RFC5246]
  static constexpr uint16_t TLS_RSA_WITH_AES_128_CBC_SHA256 =
      0x003C;  // [RFC5246]
  static constexpr uint16_t TLS_RSA_WITH_AES_256_CBC_SHA256 =
      0x003D;  // [RFC5246]
  static constexpr uint16_t TLS_DH_DSS_WITH_AES_128_CBC_SHA256 =
      0x003E;  // [RFC5246]
  static constexpr uint16_t TLS_DH_RSA_WITH_AES_128_CBC_SHA256 =
      0x003F;  // [RFC5246]
  static constexpr uint16_t TLS_DHE_DSS_WITH_AES_128_CBC_SHA256 =
      0x0040;  // [RFC5246]
  static constexpr uint16_t TLS_RSA_WITH_CAMELLIA_128_CBC_SHA =
      0x0041;  // [RFC5932]
  static constexpr uint16_t TLS_DH_DSS_WITH_CAMELLIA_128_CBC_SHA =
      0x0042;  // [RFC5932]
  static constexpr uint16_t TLS_DH_RSA_WITH_CAMELLIA_128_CBC_SHA =
      0x0043;  // [RFC5932]
  static constexpr uint16_t TLS_DHE_DSS_WITH_CAMELLIA_128_CBC_SHA =
      0x0044;  // [RFC5932]
  static constexpr uint16_t TLS_DHE_RSA_WITH_CAMELLIA_128_CBC_SHA =
      0x0045;  // [RFC5932]
  static constexpr uint16_t TLS_DH_anon_WITH_CAMELLIA_128_CBC_SHA =
      0x0046;  // [RFC5932]
  // 0x0047-4F 	Reserved to avoid conflicts with deployed implementations
  // [Pasi_Eronen] 0x0050-58 	Reserved to avoid conflicts
  // [Pasi Eronen, <pasi.eronen&nokia.com>, 2008-04-04. 2008-04-04] 0x0059-5C
  // Reserved to avoid conflicts with deployed implementations
  // [Pasi_Eronen] 0x0060-66 	Reserved to avoid conflicts with widely deployed
  // implementations 			[Pasi_Eronen]
  static constexpr uint16_t TLS_DHE_RSA_WITH_AES_128_CBC_SHA256 =
      0x0067;  // [RFC5246]
  static constexpr uint16_t TLS_DH_DSS_WITH_AES_256_CBC_SHA256 =
      0x0068;  // [RFC5246]
  static constexpr uint16_t TLS_DH_RSA_WITH_AES_256_CBC_SHA256 =
      0x0069;  // [RFC5246]
  static constexpr uint16_t TLS_DHE_DSS_WITH_AES_256_CBC_SHA256 =
      0x006A;  // [RFC5246]
  static constexpr uint16_t TLS_DHE_RSA_WITH_AES_256_CBC_SHA256 =
      0x006B;  // [RFC5246]
  static constexpr uint16_t TLS_DH_anon_WITH_AES_128_CBC_SHA256 =
      0x006C;  // [RFC5246]
  static constexpr uint16_t TLS_DH_anon_WITH_AES_256_CBC_SHA256 =
      0x006D;  // [RFC5246]
  static constexpr uint16_t TLS_RSA_WITH_CAMELLIA_256_CBC_SHA =
      0x0084;  // [RFC5932]
  static constexpr uint16_t TLS_DH_DSS_WITH_CAMELLIA_256_CBC_SHA =
      0x0085;  // [RFC5932]
  static constexpr uint16_t TLS_DH_RSA_WITH_CAMELLIA_256_CBC_SHA =
      0x0086;  // [RFC5932]
  static constexpr uint16_t TLS_DHE_DSS_WITH_CAMELLIA_256_CBC_SHA =
      0x0087;  // [RFC5932]
  static constexpr uint16_t TLS_DHE_RSA_WITH_CAMELLIA_256_CBC_SHA =
      0x0088;  // [RFC5932]
  static constexpr uint16_t TLS_DH_anon_WITH_CAMELLIA_256_CBC_SHA =
      0x0089;  // [RFC5932]
  static constexpr uint16_t TLS_PSK_WITH_RC4_128_SHA =
      0x008A;  // [RFC4279][RFC6347]
  static constexpr uint16_t TLS_PSK_WITH_3DES_EDE_CBC_SHA =
      0x008B;                                                       // [RFC4279]
  static constexpr uint16_t TLS_PSK_WITH_AES_128_CBC_SHA = 0x008C;  // [RFC4279]
  static constexpr uint16_t TLS_PSK_WITH_AES_256_CBC_SHA = 0x008D;  // [RFC4279]
  static constexpr uint16_t TLS_DHE_PSK_WITH_RC4_128_SHA =
      0x008E;  // [RFC4279][RFC6347]
  static constexpr uint16_t TLS_DHE_PSK_WITH_3DES_EDE_CBC_SHA =
      0x008F;  // [RFC4279]
  static constexpr uint16_t TLS_DHE_PSK_WITH_AES_128_CBC_SHA =
      0x0090;  // [RFC4279]
  static constexpr uint16_t TLS_DHE_PSK_WITH_AES_256_CBC_SHA =
      0x0091;  // [RFC4279]
  static constexpr uint16_t TLS_RSA_PSK_WITH_RC4_128_SHA =
      0x0092;  // [RFC4279][RFC6347]
  static constexpr uint16_t TLS_RSA_PSK_WITH_3DES_EDE_CBC_SHA =
      0x0093;  // [RFC4279]
  static constexpr uint16_t TLS_RSA_PSK_WITH_AES_128_CBC_SHA =
      0x0094;  // [RFC4279]
  static constexpr uint16_t TLS_RSA_PSK_WITH_AES_256_CBC_SHA =
      0x0095;                                                       // [RFC4279]
  static constexpr uint16_t TLS_RSA_WITH_SEED_CBC_SHA = 0x0096;     // [RFC4162]
  static constexpr uint16_t TLS_DH_DSS_WITH_SEED_CBC_SHA = 0x0097;  // [RFC4162]
  static constexpr uint16_t TLS_DH_RSA_WITH_SEED_CBC_SHA = 0x0098;  // [RFC4162]
  static constexpr uint16_t TLS_DHE_DSS_WITH_SEED_CBC_SHA =
      0x0099;  // [RFC4162]
  static constexpr uint16_t TLS_DHE_RSA_WITH_SEED_CBC_SHA =
      0x009A;  // [RFC4162]
  static constexpr uint16_t TLS_DH_anon_WITH_SEED_CBC_SHA =
      0x009B;  // [RFC4162]
  static constexpr uint16_t TLS_RSA_WITH_AES_128_GCM_SHA256 =
      0x009C;  // [RFC5288]
  static constexpr uint16_t TLS_RSA_WITH_AES_256_GCM_SHA384 =
      0x009D;  // [RFC5288]
  static constexpr uint16_t TLS_DHE_RSA_WITH_AES_128_GCM_SHA256 =
      0x009E;  // [RFC5288]
  static constexpr uint16_t TLS_DHE_RSA_WITH_AES_256_GCM_SHA384 =
      0x009F;  // [RFC5288]
  static constexpr uint16_t TLS_DH_RSA_WITH_AES_128_GCM_SHA256 =
      0x00A0;  // [RFC5288]
  static constexpr uint16_t TLS_DH_RSA_WITH_AES_256_GCM_SHA384 =
      0x00A1;  // [RFC5288]
  static constexpr uint16_t TLS_DHE_DSS_WITH_AES_128_GCM_SHA256 =
      0x00A2;  // [RFC5288]
  static constexpr uint16_t TLS_DHE_DSS_WITH_AES_256_GCM_SHA384 =
      0x00A3;  // [RFC5288]
  static constexpr uint16_t TLS_DH_DSS_WITH_AES_128_GCM_SHA256 =
      0x00A4;  // [RFC5288]
  static constexpr uint16_t TLS_DH_DSS_WITH_AES_256_GCM_SHA384 =
      0x00A5;  // [RFC5288]
  static constexpr uint16_t TLS_DH_anon_WITH_AES_128_GCM_SHA256 =
      0x00A6;  // [RFC5288]
  static constexpr uint16_t TLS_DH_anon_WITH_AES_256_GCM_SHA384 =
      0x00A7;  // [RFC5288]
  static constexpr uint16_t TLS_PSK_WITH_AES_128_GCM_SHA256 =
      0x00A8;  // [RFC5487]
  static constexpr uint16_t TLS_PSK_WITH_AES_256_GCM_SHA384 =
      0x00A9;  // [RFC5487]
  static constexpr uint16_t TLS_DHE_PSK_WITH_AES_128_GCM_SHA256 =
      0x00AA;  // [RFC5487]
  static constexpr uint16_t TLS_DHE_PSK_WITH_AES_256_GCM_SHA384 =
      0x00AB;  // [RFC5487]
  static constexpr uint16_t TLS_RSA_PSK_WITH_AES_128_GCM_SHA256 =
      0x00AC;  // [RFC5487]
  static constexpr uint16_t TLS_RSA_PSK_WITH_AES_256_GCM_SHA384 =
      0x00AD;  // [RFC5487]
  static constexpr uint16_t TLS_PSK_WITH_AES_128_CBC_SHA256 =
      0x00AE;  // [RFC5487]
  static constexpr uint16_t TLS_PSK_WITH_AES_256_CBC_SHA384 =
      0x00AF;                                                   // [RFC5487]
  static constexpr uint16_t TLS_PSK_WITH_NULL_SHA256 = 0x00B0;  // [RFC5487]
  static constexpr uint16_t TLS_PSK_WITH_NULL_SHA384 = 0x00B1;  // [RFC5487]
  static constexpr uint16_t TLS_DHE_PSK_WITH_AES_128_CBC_SHA256 =
      0x00B2;  // [RFC5487]
  static constexpr uint16_t TLS_DHE_PSK_WITH_AES_256_CBC_SHA384 =
      0x00B3;                                                       // [RFC5487]
  static constexpr uint16_t TLS_DHE_PSK_WITH_NULL_SHA256 = 0x00B4;  // [RFC5487]
  static constexpr uint16_t TLS_DHE_PSK_WITH_NULL_SHA384 = 0x00B5;  // [RFC5487]
  static constexpr uint16_t TLS_RSA_PSK_WITH_AES_128_CBC_SHA256 =
      0x00B6;  // [RFC5487]
  static constexpr uint16_t TLS_RSA_PSK_WITH_AES_256_CBC_SHA384 =
      0x00B7;                                                       // [RFC5487]
  static constexpr uint16_t TLS_RSA_PSK_WITH_NULL_SHA256 = 0x00B8;  // [RFC5487]
  static constexpr uint16_t TLS_RSA_PSK_WITH_NULL_SHA384 = 0x00B9;  // [RFC5487]
  static constexpr uint16_t TLS_RSA_WITH_CAMELLIA_128_CBC_SHA256 =
      0x00BA;  // [RFC5932]
  static constexpr uint16_t TLS_DH_DSS_WITH_CAMELLIA_128_CBC_SHA256 =
      0x00BB;  // [RFC5932]
  static constexpr uint16_t TLS_DH_RSA_WITH_CAMELLIA_128_CBC_SHA256 =
      0x00BC;  // [RFC5932]
  static constexpr uint16_t TLS_DHE_DSS_WITH_CAMELLIA_128_CBC_SHA256 =
      0x00BD;  // [RFC5932]
  static constexpr uint16_t TLS_DHE_RSA_WITH_CAMELLIA_128_CBC_SHA256 =
      0x00BE;  // [RFC5932]
  static constexpr uint16_t TLS_DH_anon_WITH_CAMELLIA_128_CBC_SHA256 =
      0x00BF;  // [RFC5932]
  static constexpr uint16_t TLS_RSA_WITH_CAMELLIA_256_CBC_SHA256 =
      0x00C0;  // [RFC5932]
  static constexpr uint16_t TLS_DH_DSS_WITH_CAMELLIA_256_CBC_SHA256 =
      0x00C1;  // [RFC5932]
  static constexpr uint16_t TLS_DH_RSA_WITH_CAMELLIA_256_CBC_SHA256 =
      0x00C2;  // [RFC5932]
  static constexpr uint16_t TLS_DHE_DSS_WITH_CAMELLIA_256_CBC_SHA256 =
      0x00C3;  // [RFC5932]
  static constexpr uint16_t TLS_DHE_RSA_WITH_CAMELLIA_256_CBC_SHA256 =
      0x00C4;  // [RFC5932]
  static constexpr uint16_t TLS_DH_anon_WITH_CAMELLIA_256_CBC_SHA256 =
      0x00C5;                                          // [RFC5932]
  static constexpr uint16_t TLS_SM4_GCM_SM3 = 0x00C6;  // [RFC8998]
  static constexpr uint16_t TLS_SM4_CCM_SM3 = 0x00C7;  // [RFC8998]
  static constexpr uint16_t TLS_EMPTY_RENEGOTIATION_INFO_SCSV =
      0x00FF;  // [RFC5746]
  // 0x0A0A 	Reserved 	[RFC8701]
  static constexpr uint16_t TLS_AES_128_GCM_SHA256 = 0x1301;        // [RFC8446]
  static constexpr uint16_t TLS_AES_256_GCM_SHA384 = 0x1302;        // [RFC8446]
  static constexpr uint16_t TLS_CHACHA20_POLY1305_SHA256 = 0x1303;  // [RFC8446]
  static constexpr uint16_t TLS_AES_128_CCM_SHA256 = 0x1304;        // [RFC8446]
  static constexpr uint16_t TLS_AES_128_CCM_8_SHA256 =
      0x1305;  // [RFC8446][IESG Action 2018-08-16]
  static constexpr uint16_t TLS_AEGIS_256_SHA384 =
      0x1306;  // [draft-irtf-cfrg-aegis-aead-00]
  static constexpr uint16_t TLS_AEGIS_128L_SHA256 =
      0x1307;  // [draft-irtf-cfrg-aegis-aead-00]
  // 0x1A1A 	Reserved 	[RFC8701]
  // 0x2A2A 	Reserved 	[RFC8701]
  // 0x3A3A 	Reserved 	[RFC8701]
  // 0x4A4A 	Reserved 	[RFC8701]
  static constexpr uint16_t TLS_FALLBACK_SCSV = 0x5600;  // [RFC7507]
  // 0x5A5A 	Reserved 	[RFC8701]
  // 0x6A6A 	Reserved 	[RFC8701]
  // 0x7A7A 	Reserved 	[RFC8701]
  // 0x8A8A 	Reserved 	[RFC8701]
  // 0x9A9A 	Reserved 	[RFC8701]
  // 0xAAAA 	Reserved 	[RFC8701]
  // 0xBABA 	Reserved 	[RFC8701]
  static constexpr uint16_t TLS_ECDH_ECDSA_WITH_NULL_SHA = 0xC001;  // [RFC8422]
  static constexpr uint16_t TLS_ECDH_ECDSA_WITH_RC4_128_SHA =
      0xC002;  // [RFC8422][RFC6347]
  static constexpr uint16_t TLS_ECDH_ECDSA_WITH_3DES_EDE_CBC_SHA =
      0xC003;  // [RFC8422]
  static constexpr uint16_t TLS_ECDH_ECDSA_WITH_AES_128_CBC_SHA =
      0xC004;  // [RFC8422]
  static constexpr uint16_t TLS_ECDH_ECDSA_WITH_AES_256_CBC_SHA =
      0xC005;  // [RFC8422]
  static constexpr uint16_t TLS_ECDHE_ECDSA_WITH_NULL_SHA =
      0xC006;  // [RFC8422]
  static constexpr uint16_t TLS_ECDHE_ECDSA_WITH_RC4_128_SHA =
      0xC007;  // [RFC8422][RFC6347]
  static constexpr uint16_t TLS_ECDHE_ECDSA_WITH_3DES_EDE_CBC_SHA =
      0xC008;  // [RFC8422]
  static constexpr uint16_t TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA =
      0xC009;  // [RFC8422]
  static constexpr uint16_t TLS_ECDHE_ECDSA_WITH_AES_256_CBC_SHA =
      0xC00A;                                                     // [RFC8422]
  static constexpr uint16_t TLS_ECDH_RSA_WITH_NULL_SHA = 0xC00B;  // [RFC8422]
  static constexpr uint16_t TLS_ECDH_RSA_WITH_RC4_128_SHA =
      0xC00C;  // [RFC8422][RFC6347]
  static constexpr uint16_t TLS_ECDH_RSA_WITH_3DES_EDE_CBC_SHA =
      0xC00D;  // [RFC8422]
  static constexpr uint16_t TLS_ECDH_RSA_WITH_AES_128_CBC_SHA =
      0xC00E;  // [RFC8422]
  static constexpr uint16_t TLS_ECDH_RSA_WITH_AES_256_CBC_SHA =
      0xC00F;                                                      // [RFC8422]
  static constexpr uint16_t TLS_ECDHE_RSA_WITH_NULL_SHA = 0xC010;  // [RFC8422]
  static constexpr uint16_t TLS_ECDHE_RSA_WITH_RC4_128_SHA =
      0xC011;  // [RFC8422][RFC6347]
  static constexpr uint16_t TLS_ECDHE_RSA_WITH_3DES_EDE_CBC_SHA =
      0xC012;  // [RFC8422]
  static constexpr uint16_t TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA =
      0xC013;  // [RFC8422]
  static constexpr uint16_t TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA =
      0xC014;                                                      // [RFC8422]
  static constexpr uint16_t TLS_ECDH_anon_WITH_NULL_SHA = 0xC015;  // [RFC8422]
  static constexpr uint16_t TLS_ECDH_anon_WITH_RC4_128_SHA =
      0xC016;  // [RFC8422][RFC6347]
  static constexpr uint16_t TLS_ECDH_anon_WITH_3DES_EDE_CBC_SHA =
      0xC017;  // [RFC8422]
  static constexpr uint16_t TLS_ECDH_anon_WITH_AES_128_CBC_SHA =
      0xC018;  // [RFC8422]
  static constexpr uint16_t TLS_ECDH_anon_WITH_AES_256_CBC_SHA =
      0xC019;  // [RFC8422]
  static constexpr uint16_t TLS_SRP_SHA_WITH_3DES_EDE_CBC_SHA =
      0xC01A;  // [RFC5054]
  static constexpr uint16_t TLS_SRP_SHA_RSA_WITH_3DES_EDE_CBC_SHA =
      0xC01B;  // [RFC5054]
  static constexpr uint16_t TLS_SRP_SHA_DSS_WITH_3DES_EDE_CBC_SHA =
      0xC01C;  // [RFC5054]
  static constexpr uint16_t TLS_SRP_SHA_WITH_AES_128_CBC_SHA =
      0xC01D;  // [RFC5054]
  static constexpr uint16_t TLS_SRP_SHA_RSA_WITH_AES_128_CBC_SHA =
      0xC01E;  // [RFC5054]
  static constexpr uint16_t TLS_SRP_SHA_DSS_WITH_AES_128_CBC_SHA =
      0xC01F;  // [RFC5054]
  static constexpr uint16_t TLS_SRP_SHA_WITH_AES_256_CBC_SHA =
      0xC020;  // [RFC5054]
  static constexpr uint16_t TLS_SRP_SHA_RSA_WITH_AES_256_CBC_SHA =
      0xC021;  // [RFC5054]
  static constexpr uint16_t TLS_SRP_SHA_DSS_WITH_AES_256_CBC_SHA =
      0xC022;  // [RFC5054]
  static constexpr uint16_t TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA256 =
      0xC023;  // [RFC5289]
  static constexpr uint16_t TLS_ECDHE_ECDSA_WITH_AES_256_CBC_SHA384 =
      0xC024;  // [RFC5289]
  static constexpr uint16_t TLS_ECDH_ECDSA_WITH_AES_128_CBC_SHA256 =
      0xC025;  // [RFC5289]
  static constexpr uint16_t TLS_ECDH_ECDSA_WITH_AES_256_CBC_SHA384 =
      0xC026;  // [RFC5289]
  static constexpr uint16_t TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA256 =
      0xC027;  // [RFC5289]
  static constexpr uint16_t TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA384 =
      0xC028;  // [RFC5289]
  static constexpr uint16_t TLS_ECDH_RSA_WITH_AES_128_CBC_SHA256 =
      0xC029;  // [RFC5289]
  static constexpr uint16_t TLS_ECDH_RSA_WITH_AES_256_CBC_SHA384 =
      0xC02A;  // [RFC5289]
  static constexpr uint16_t TLS_ECDHE_ECDSA_WITH_AES_128_GCM_SHA256 =
      0xC02B;  // [RFC5289]
  static constexpr uint16_t TLS_ECDHE_ECDSA_WITH_AES_256_GCM_SHA384 =
      0xC02C;  // [RFC5289]
  static constexpr uint16_t TLS_ECDH_ECDSA_WITH_AES_128_GCM_SHA256 =
      0xC02D;  // [RFC5289]
  static constexpr uint16_t TLS_ECDH_ECDSA_WITH_AES_256_GCM_SHA384 =
      0xC02E;  // [RFC5289]
  static constexpr uint16_t TLS_ECDHE_RSA_WITH_AES_128_GCM_SHA256 =
      0xC02F;  // [RFC5289]
  static constexpr uint16_t TLS_ECDHE_RSA_WITH_AES_256_GCM_SHA384 =
      0xC030;  // [RFC5289]
  static constexpr uint16_t TLS_ECDH_RSA_WITH_AES_128_GCM_SHA256 =
      0xC031;  // [RFC5289]
  static constexpr uint16_t TLS_ECDH_RSA_WITH_AES_256_GCM_SHA384 =
      0xC032;  // [RFC5289]
  static constexpr uint16_t TLS_ECDHE_PSK_WITH_RC4_128_SHA =
      0xC033;  // [RFC5489][RFC6347]
  static constexpr uint16_t TLS_ECDHE_PSK_WITH_3DES_EDE_CBC_SHA =
      0xC034;  // [RFC5489]
  static constexpr uint16_t TLS_ECDHE_PSK_WITH_AES_128_CBC_SHA =
      0xC035;  // [RFC5489]
  static constexpr uint16_t TLS_ECDHE_PSK_WITH_AES_256_CBC_SHA =
      0xC036;  // [RFC5489]
  static constexpr uint16_t TLS_ECDHE_PSK_WITH_AES_128_CBC_SHA256 =
      0xC037;  // [RFC5489]
  static constexpr uint16_t TLS_ECDHE_PSK_WITH_AES_256_CBC_SHA384 =
      0xC038;                                                      // [RFC5489]
  static constexpr uint16_t TLS_ECDHE_PSK_WITH_NULL_SHA = 0xC039;  // [RFC5489]
  static constexpr uint16_t TLS_ECDHE_PSK_WITH_NULL_SHA256 =
      0xC03A;  // [RFC5489]
  static constexpr uint16_t TLS_ECDHE_PSK_WITH_NULL_SHA384 =
      0xC03B;  // [RFC5489]
  static constexpr uint16_t TLS_RSA_WITH_ARIA_128_CBC_SHA256 =
      0xC03C;  // [RFC6209]
  static constexpr uint16_t TLS_RSA_WITH_ARIA_256_CBC_SHA384 =
      0xC03D;  // [RFC6209]
  static constexpr uint16_t TLS_DH_DSS_WITH_ARIA_128_CBC_SHA256 =
      0xC03E;  // [RFC6209]
  static constexpr uint16_t TLS_DH_DSS_WITH_ARIA_256_CBC_SHA384 =
      0xC03F;  // [RFC6209]
  static constexpr uint16_t TLS_DH_RSA_WITH_ARIA_128_CBC_SHA256 =
      0xC040;  // [RFC6209]
  static constexpr uint16_t TLS_DH_RSA_WITH_ARIA_256_CBC_SHA384 =
      0xC041;  // [RFC6209]
  static constexpr uint16_t TLS_DHE_DSS_WITH_ARIA_128_CBC_SHA256 =
      0xC042;  // [RFC6209]
  static constexpr uint16_t TLS_DHE_DSS_WITH_ARIA_256_CBC_SHA384 =
      0xC043;  // [RFC6209]
  static constexpr uint16_t TLS_DHE_RSA_WITH_ARIA_128_CBC_SHA256 =
      0xC044;  // [RFC6209]
  static constexpr uint16_t TLS_DHE_RSA_WITH_ARIA_256_CBC_SHA384 =
      0xC045;  // [RFC6209]
  static constexpr uint16_t TLS_DH_anon_WITH_ARIA_128_CBC_SHA256 =
      0xC046;  // [RFC6209]
  static constexpr uint16_t TLS_DH_anon_WITH_ARIA_256_CBC_SHA384 =
      0xC047;  // [RFC6209]
  static constexpr uint16_t TLS_ECDHE_ECDSA_WITH_ARIA_128_CBC_SHA256 =
      0xC048;  // [RFC6209]
  static constexpr uint16_t TLS_ECDHE_ECDSA_WITH_ARIA_256_CBC_SHA384 =
      0xC049;  // [RFC6209]
  static constexpr uint16_t TLS_ECDH_ECDSA_WITH_ARIA_128_CBC_SHA256 =
      0xC04A;  // [RFC6209]
  static constexpr uint16_t TLS_ECDH_ECDSA_WITH_ARIA_256_CBC_SHA384 =
      0xC04B;  // [RFC6209]
  static constexpr uint16_t TLS_ECDHE_RSA_WITH_ARIA_128_CBC_SHA256 =
      0xC04C;  // [RFC6209]
  static constexpr uint16_t TLS_ECDHE_RSA_WITH_ARIA_256_CBC_SHA384 =
      0xC04D;  // [RFC6209]
  static constexpr uint16_t TLS_ECDH_RSA_WITH_ARIA_128_CBC_SHA256 =
      0xC04E;  // [RFC6209]
  static constexpr uint16_t TLS_ECDH_RSA_WITH_ARIA_256_CBC_SHA384 =
      0xC04F;  // [RFC6209]
  static constexpr uint16_t TLS_RSA_WITH_ARIA_128_GCM_SHA256 =
      0xC050;  // [RFC6209]
  static constexpr uint16_t TLS_RSA_WITH_ARIA_256_GCM_SHA384 =
      0xC051;  // [RFC6209]
  static constexpr uint16_t TLS_DHE_RSA_WITH_ARIA_128_GCM_SHA256 =
      0xC052;  // [RFC6209]
  static constexpr uint16_t TLS_DHE_RSA_WITH_ARIA_256_GCM_SHA384 =
      0xC053;  // [RFC6209]
  static constexpr uint16_t TLS_DH_RSA_WITH_ARIA_128_GCM_SHA256 =
      0xC054;  // [RFC6209]
  static constexpr uint16_t TLS_DH_RSA_WITH_ARIA_256_GCM_SHA384 =
      0xC055;  // [RFC6209]
  static constexpr uint16_t TLS_DHE_DSS_WITH_ARIA_128_GCM_SHA256 =
      0xC056;  // [RFC6209]
  static constexpr uint16_t TLS_DHE_DSS_WITH_ARIA_256_GCM_SHA384 =
      0xC057;  // [RFC6209]
  static constexpr uint16_t TLS_DH_DSS_WITH_ARIA_128_GCM_SHA256 =
      0xC058;  // [RFC6209]
  static constexpr uint16_t TLS_DH_DSS_WITH_ARIA_256_GCM_SHA384 =
      0xC059;  // [RFC6209]
  static constexpr uint16_t TLS_DH_anon_WITH_ARIA_128_GCM_SHA256 =
      0xC05A;  // [RFC6209]
  static constexpr uint16_t TLS_DH_anon_WITH_ARIA_256_GCM_SHA384 =
      0xC05B;  // [RFC6209]
  static constexpr uint16_t TLS_ECDHE_ECDSA_WITH_ARIA_128_GCM_SHA256 =
      0xC05C;  // [RFC6209]
  static constexpr uint16_t TLS_ECDHE_ECDSA_WITH_ARIA_256_GCM_SHA384 =
      0xC05D;  // [RFC6209]
  static constexpr uint16_t TLS_ECDH_ECDSA_WITH_ARIA_128_GCM_SHA256 =
      0xC05E;  // [RFC6209]
  static constexpr uint16_t TLS_ECDH_ECDSA_WITH_ARIA_256_GCM_SHA384 =
      0xC05F;  // [RFC6209]
  static constexpr uint16_t TLS_ECDHE_RSA_WITH_ARIA_128_GCM_SHA256 =
      0xC060;  // [RFC6209]
  static constexpr uint16_t TLS_ECDHE_RSA_WITH_ARIA_256_GCM_SHA384 =
      0xC061;  // [RFC6209]
  static constexpr uint16_t TLS_ECDH_RSA_WITH_ARIA_128_GCM_SHA256 =
      0xC062;  // [RFC6209]
  static constexpr uint16_t TLS_ECDH_RSA_WITH_ARIA_256_GCM_SHA384 =
      0xC063;  // [RFC6209]
  static constexpr uint16_t TLS_PSK_WITH_ARIA_128_CBC_SHA256 =
      0xC064;  // [RFC6209]
  static constexpr uint16_t TLS_PSK_WITH_ARIA_256_CBC_SHA384 =
      0xC065;  // [RFC6209]
  static constexpr uint16_t TLS_DHE_PSK_WITH_ARIA_128_CBC_SHA256 =
      0xC066;  // [RFC6209]
  static constexpr uint16_t TLS_DHE_PSK_WITH_ARIA_256_CBC_SHA384 =
      0xC067;  // [RFC6209]
  static constexpr uint16_t TLS_RSA_PSK_WITH_ARIA_128_CBC_SHA256 =
      0xC068;  // [RFC6209]
  static constexpr uint16_t TLS_RSA_PSK_WITH_ARIA_256_CBC_SHA384 =
      0xC069;  // [RFC6209]
  static constexpr uint16_t TLS_PSK_WITH_ARIA_128_GCM_SHA256 =
      0xC06A;  // [RFC6209]
  static constexpr uint16_t TLS_PSK_WITH_ARIA_256_GCM_SHA384 =
      0xC06B;  // [RFC6209]
  static constexpr uint16_t TLS_DHE_PSK_WITH_ARIA_128_GCM_SHA256 =
      0xC06C;  // [RFC6209]
  static constexpr uint16_t TLS_DHE_PSK_WITH_ARIA_256_GCM_SHA384 =
      0xC06D;  // [RFC6209]
  static constexpr uint16_t TLS_RSA_PSK_WITH_ARIA_128_GCM_SHA256 =
      0xC06E;  // [RFC6209]
  static constexpr uint16_t TLS_RSA_PSK_WITH_ARIA_256_GCM_SHA384 =
      0xC06F;  // [RFC6209]
  static constexpr uint16_t TLS_ECDHE_PSK_WITH_ARIA_128_CBC_SHA256 =
      0xC070;  // [RFC6209]
  static constexpr uint16_t TLS_ECDHE_PSK_WITH_ARIA_256_CBC_SHA384 =
      0xC071;  // [RFC6209]
  static constexpr uint16_t TLS_ECDHE_ECDSA_WITH_CAMELLIA_128_CBC_SHA256 =
      0xC072;  // [RFC6367]
  static constexpr uint16_t TLS_ECDHE_ECDSA_WITH_CAMELLIA_256_CBC_SHA384 =
      0xC073;  // [RFC6367]
  static constexpr uint16_t TLS_ECDH_ECDSA_WITH_CAMELLIA_128_CBC_SHA256 =
      0xC074;  // [RFC6367]
  static constexpr uint16_t TLS_ECDH_ECDSA_WITH_CAMELLIA_256_CBC_SHA384 =
      0xC075;  // [RFC6367]
  static constexpr uint16_t TLS_ECDHE_RSA_WITH_CAMELLIA_128_CBC_SHA256 =
      0xC076;  // [RFC6367]
  static constexpr uint16_t TLS_ECDHE_RSA_WITH_CAMELLIA_256_CBC_SHA384 =
      0xC077;  // [RFC6367]
  static constexpr uint16_t TLS_ECDH_RSA_WITH_CAMELLIA_128_CBC_SHA256 =
      0xC078;  // [RFC6367]
  static constexpr uint16_t TLS_ECDH_RSA_WITH_CAMELLIA_256_CBC_SHA384 =
      0xC079;  // [RFC6367]
  static constexpr uint16_t TLS_RSA_WITH_CAMELLIA_128_GCM_SHA256 =
      0xC07A;  // [RFC6367]
  static constexpr uint16_t TLS_RSA_WITH_CAMELLIA_256_GCM_SHA384 =
      0xC07B;  // [RFC6367]
  static constexpr uint16_t TLS_DHE_RSA_WITH_CAMELLIA_128_GCM_SHA256 =
      0xC07C;  // [RFC6367]
  static constexpr uint16_t TLS_DHE_RSA_WITH_CAMELLIA_256_GCM_SHA384 =
      0xC07D;  // [RFC6367]
  static constexpr uint16_t TLS_DH_RSA_WITH_CAMELLIA_128_GCM_SHA256 =
      0xC07E;  // [RFC6367]
  static constexpr uint16_t TLS_DH_RSA_WITH_CAMELLIA_256_GCM_SHA384 =
      0xC07F;  // [RFC6367]
  static constexpr uint16_t TLS_DHE_DSS_WITH_CAMELLIA_128_GCM_SHA256 =
      0xC080;  // [RFC6367]
  static constexpr uint16_t TLS_DHE_DSS_WITH_CAMELLIA_256_GCM_SHA384 =
      0xC081;  // [RFC6367]
  static constexpr uint16_t TLS_DH_DSS_WITH_CAMELLIA_128_GCM_SHA256 =
      0xC082;  // [RFC6367]
  static constexpr uint16_t TLS_DH_DSS_WITH_CAMELLIA_256_GCM_SHA384 =
      0xC083;  // [RFC6367]
  static constexpr uint16_t TLS_DH_anon_WITH_CAMELLIA_128_GCM_SHA256 =
      0xC084;  // [RFC6367]
  static constexpr uint16_t TLS_DH_anon_WITH_CAMELLIA_256_GCM_SHA384 =
      0xC085;  // [RFC6367]
  static constexpr uint16_t TLS_ECDHE_ECDSA_WITH_CAMELLIA_128_GCM_SHA256 =
      0xC086;  // [RFC6367]
  static constexpr uint16_t TLS_ECDHE_ECDSA_WITH_CAMELLIA_256_GCM_SHA384 =
      0xC087;  // [RFC6367]
  static constexpr uint16_t TLS_ECDH_ECDSA_WITH_CAMELLIA_128_GCM_SHA256 =
      0xC088;  // [RFC6367]
  static constexpr uint16_t TLS_ECDH_ECDSA_WITH_CAMELLIA_256_GCM_SHA384 =
      0xC089;  // [RFC6367]
  static constexpr uint16_t TLS_ECDHE_RSA_WITH_CAMELLIA_128_GCM_SHA256 =
      0xC08A;  // [RFC6367]
  static constexpr uint16_t TLS_ECDHE_RSA_WITH_CAMELLIA_256_GCM_SHA384 =
      0xC08B;  // [RFC6367]
  static constexpr uint16_t TLS_ECDH_RSA_WITH_CAMELLIA_128_GCM_SHA256 =
      0xC08C;  // [RFC6367]
  static constexpr uint16_t TLS_ECDH_RSA_WITH_CAMELLIA_256_GCM_SHA384 =
      0xC08D;  // [RFC6367]
  static constexpr uint16_t TLS_PSK_WITH_CAMELLIA_128_GCM_SHA256 =
      0xC08E;  // [RFC6367]
  static constexpr uint16_t TLS_PSK_WITH_CAMELLIA_256_GCM_SHA384 =
      0xC08F;  // [RFC6367]
  static constexpr uint16_t TLS_DHE_PSK_WITH_CAMELLIA_128_GCM_SHA256 =
      0xC090;  // [RFC6367]
  static constexpr uint16_t TLS_DHE_PSK_WITH_CAMELLIA_256_GCM_SHA384 =
      0xC091;  // [RFC6367]
  static constexpr uint16_t TLS_RSA_PSK_WITH_CAMELLIA_128_GCM_SHA256 =
      0xC092;  // [RFC6367]
  static constexpr uint16_t TLS_RSA_PSK_WITH_CAMELLIA_256_GCM_SHA384 =
      0xC093;  // [RFC6367]
  static constexpr uint16_t TLS_PSK_WITH_CAMELLIA_128_CBC_SHA256 =
      0xC094;  // [RFC6367]
  static constexpr uint16_t TLS_PSK_WITH_CAMELLIA_256_CBC_SHA384 =
      0xC095;  // [RFC6367]
  static constexpr uint16_t TLS_DHE_PSK_WITH_CAMELLIA_128_CBC_SHA256 =
      0xC096;  // [RFC6367]
  static constexpr uint16_t TLS_DHE_PSK_WITH_CAMELLIA_256_CBC_SHA384 =
      0xC097;  // [RFC6367]
  static constexpr uint16_t TLS_RSA_PSK_WITH_CAMELLIA_128_CBC_SHA256 =
      0xC098;  // [RFC6367]
  static constexpr uint16_t TLS_RSA_PSK_WITH_CAMELLIA_256_CBC_SHA384 =
      0xC099;  // [RFC6367]
  static constexpr uint16_t TLS_ECDHE_PSK_WITH_CAMELLIA_128_CBC_SHA256 =
      0xC09A;  // [RFC6367]
  static constexpr uint16_t TLS_ECDHE_PSK_WITH_CAMELLIA_256_CBC_SHA384 =
      0xC09B;                                                       // [RFC6367]
  static constexpr uint16_t TLS_RSA_WITH_AES_128_CCM = 0xC09C;      // [RFC6655]
  static constexpr uint16_t TLS_RSA_WITH_AES_256_CCM = 0xC09D;      // [RFC6655]
  static constexpr uint16_t TLS_DHE_RSA_WITH_AES_128_CCM = 0xC09E;  // [RFC6655]
  static constexpr uint16_t TLS_DHE_RSA_WITH_AES_256_CCM = 0xC09F;  // [RFC6655]
  static constexpr uint16_t TLS_RSA_WITH_AES_128_CCM_8 = 0xC0A0;    // [RFC6655]
  static constexpr uint16_t TLS_RSA_WITH_AES_256_CCM_8 = 0xC0A1;    // [RFC6655]
  static constexpr uint16_t TLS_DHE_RSA_WITH_AES_128_CCM_8 =
      0xC0A2;  // [RFC6655]
  static constexpr uint16_t TLS_DHE_RSA_WITH_AES_256_CCM_8 =
      0xC0A3;                                                       // [RFC6655]
  static constexpr uint16_t TLS_PSK_WITH_AES_128_CCM = 0xC0A4;      // [RFC6655]
  static constexpr uint16_t TLS_PSK_WITH_AES_256_CCM = 0xC0A5;      // [RFC6655]
  static constexpr uint16_t TLS_DHE_PSK_WITH_AES_128_CCM = 0xC0A6;  // [RFC6655]
  static constexpr uint16_t TLS_DHE_PSK_WITH_AES_256_CCM = 0xC0A7;  // [RFC6655]
  static constexpr uint16_t TLS_PSK_WITH_AES_128_CCM_8 = 0xC0A8;    // [RFC6655]
  static constexpr uint16_t TLS_PSK_WITH_AES_256_CCM_8 = 0xC0A9;    // [RFC6655]
  static constexpr uint16_t TLS_PSK_DHE_WITH_AES_128_CCM_8 =
      0xC0AA;  // [RFC6655]
  static constexpr uint16_t TLS_PSK_DHE_WITH_AES_256_CCM_8 =
      0xC0AB;  // [RFC6655]
  static constexpr uint16_t TLS_ECDHE_ECDSA_WITH_AES_128_CCM =
      0xC0AC;  // [RFC7251]
  static constexpr uint16_t TLS_ECDHE_ECDSA_WITH_AES_256_CCM =
      0xC0AD;  // [RFC7251]
  static constexpr uint16_t TLS_ECDHE_ECDSA_WITH_AES_128_CCM_8 =
      0xC0AE;  // [RFC7251]
  static constexpr uint16_t TLS_ECDHE_ECDSA_WITH_AES_256_CCM_8 =
      0xC0AF;  // [RFC7251]
  static constexpr uint16_t TLS_ECCPWD_WITH_AES_128_GCM_SHA256 =
      0xC0B0;  // [RFC8492]
  static constexpr uint16_t TLS_ECCPWD_WITH_AES_256_GCM_SHA384 =
      0xC0B1;  // [RFC8492]
  static constexpr uint16_t TLS_ECCPWD_WITH_AES_128_CCM_SHA256 =
      0xC0B2;  // [RFC8492]
  static constexpr uint16_t TLS_ECCPWD_WITH_AES_256_CCM_SHA384 =
      0xC0B3;                                            // [RFC8492]
  static constexpr uint16_t TLS_SHA256_SHA256 = 0xC0B4;  // [RFC9150]
  static constexpr uint16_t TLS_SHA384_SHA384 = 0xC0B5;  // [RFC9150]
  static constexpr uint16_t TLS_GOSTR341112_256_WITH_KUZNYECHIK_CTR_OMAC =
      0xC100;  // [RFC9189]
  static constexpr uint16_t TLS_GOSTR341112_256_WITH_MAGMA_CTR_OMAC =
      0xC101;  // [RFC9189]
  static constexpr uint16_t TLS_GOSTR341112_256_WITH_28147_CNT_IMIT =
      0xC102;  // [RFC9189]
  static constexpr uint16_t TLS_GOSTR341112_256_WITH_KUZNYECHIK_MGM_L =
      0xC103;  // [RFC9367]
  static constexpr uint16_t TLS_GOSTR341112_256_WITH_MAGMA_MGM_L =
      0xC104;  // [RFC9367]
  static constexpr uint16_t TLS_GOSTR341112_256_WITH_KUZNYECHIK_MGM_S =
      0xC105;  // [RFC9367]
  static constexpr uint16_t TLS_GOSTR341112_256_WITH_MAGMA_MGM_S =
      0xC106;  // [RFC9367]
  // 0xCACA 	Reserved 	[RFC8701]
  static constexpr uint16_t TLS_ECDHE_RSA_WITH_CHACHA20_POLY1305_SHA256 =
      0xCCA8;  // [RFC7905]
  static constexpr uint16_t TLS_ECDHE_ECDSA_WITH_CHACHA20_POLY1305_SHA256 =
      0xCCA9;  // [RFC7905]
  static constexpr uint16_t TLS_DHE_RSA_WITH_CHACHA20_POLY1305_SHA256 =
      0xCCAA;  // [RFC7905]
  static constexpr uint16_t TLS_PSK_WITH_CHACHA20_POLY1305_SHA256 =
      0xCCAB;  // [RFC7905]
  static constexpr uint16_t TLS_ECDHE_PSK_WITH_CHACHA20_POLY1305_SHA256 =
      0xCCAC;  // [RFC7905]
  static constexpr uint16_t TLS_DHE_PSK_WITH_CHACHA20_POLY1305_SHA256 =
      0xCCAD;  // [RFC7905]
  static constexpr uint16_t TLS_RSA_PSK_WITH_CHACHA20_POLY1305_SHA256 =
      0xCCAE;  // [RFC7905]
  static constexpr uint16_t TLS_ECDHE_PSK_WITH_AES_128_GCM_SHA256 =
      0xD001;  // [RFC8442]
  static constexpr uint16_t TLS_ECDHE_PSK_WITH_AES_256_GCM_SHA384 =
      0xD002;  // [RFC8442]
  static constexpr uint16_t TLS_ECDHE_PSK_WITH_AES_128_CCM_8_SHA256 =
      0xD003;  // [RFC8442]
  static constexpr uint16_t TLS_ECDHE_PSK_WITH_AES_128_CCM_SHA256 =
      0xD005;  // [RFC8442]
  // 0xDADA 	Reserved 	[RFC8701]
  // 0xEAEA 	Reserved 	[RFC8701]
  // 0xFAFA 	Reserved 	[RFC8701]
  // 0xFEFE-FF 	Reserved to avoid conflicts with widely deployed implementations
  // [Pasi_Eronen] 0xFF00-FF 	Reserved for Private Use
  // [RFC8446]
};

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
  uint16_t length;
  NamedGroup groups[];
};

struct SupportedVersionList {
  uint8_t length;  // byte size
  VersionType versions[];
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
  static constexpr uint8_t server_hello_done = 14;
  static constexpr uint8_t certificate_verify = 15;
  static constexpr uint8_t finished = 20;
  static constexpr uint8_t key_update = 24;
  static constexpr uint8_t message_hash = 254;
  static constexpr uint8_t undefined = 255;
};

struct HelloRetryRequestSpecialValue {
  static constexpr const std::array<uint8_t, 24> head = {
      0xCF, 0x21, 0xAD, 0x74, 0xE5, 0x9A, 0x61, 0x11, 0xBE, 0x1D, 0x8C, 0x02,
      0x1E, 0x65, 0xB8, 0x91, 0xC2, 0xA2, 0x11, 0x16, 0x7A, 0xBB, 0x8C, 0x5E};
  static constexpr const uintptr_t tail_offset = sizeof(head);

  static constexpr const std::array<uint8_t, 8> tail_1_3 = {
      0x07, 0x9E, 0x09, 0xE2, 0xC8, 0xA8, 0x33, 0x9C};
  static constexpr const std::array<uint8_t, 8> tail_1_2 = {
      0x07, 0x9E, 0x09, 0xE2, 0xC8, 0xA8, 0x33, 0x9C};
  static constexpr const std::array<uint8_t, 8> tail_1_1 = {
      0x07, 0x9E, 0x09, 0xE2, 0xC8, 0xA8, 0x33, 0x9C};
};

struct RawHandshake {
  uint8_t msg_type; /* handshake type */
  uint24_t length;  /* remaining bytes in message */
  uint8_t data[];
};

struct RawClientHello {
  struct FixedLengthHeader {
    uint16_t legacy_version;
    uint8_t random[32];
  } fixed_length_header;
  RawSessionID legacy_session_id;
  // CipherSuites cipher_suites;
  //  CompressionMethods legacy_compression_methods;
  //  extension::Extensions extensions;
};

struct RawServerHello {
  struct FixedLengthHeader {
    uint16_t legacy_version;
    uint8_t random[32];
  } fixed_length_header;
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