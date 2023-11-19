#include <boost/algorithm/string/predicate.hpp>
#include <boost/endian/conversion.hpp>
#include <chrono>
#include <iostream>
#include <vector>

#include "logging.hpp"
#include "network.hpp"
#include "tls_message_decoder.hpp"
#include "tls_message_encoder.hpp"
#include "tls_security_policy.hpp"

namespace hood_proxy {
namespace tls {

const std::set<CipherSuite> SecurityPolicy::allowed_cipher_suites = {
    protocol::CipherSuiteValue::TLS_ECDHE_RSA_WITH_AES_128_GCM_SHA256,
    protocol::CipherSuiteValue::TLS_ECDHE_RSA_WITH_AES_256_GCM_SHA384,
    protocol::CipherSuiteValue::TLS_ECDHE_ECDSA_WITH_AES_128_GCM_SHA256,
    protocol::CipherSuiteValue::TLS_ECDHE_ECDSA_WITH_AES_256_GCM_SHA384,
    protocol::CipherSuiteValue::TLS_AES_128_GCM_SHA256,
    protocol::CipherSuiteValue::TLS_AES_256_GCM_SHA384,
    protocol::CipherSuiteValue::TLS_CHACHA20_POLY1305_SHA256};

}  // namespace tls
}  // namespace hood_proxy