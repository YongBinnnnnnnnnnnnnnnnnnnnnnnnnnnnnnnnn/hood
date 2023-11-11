#ifndef HOOD_PROXY_TLS_MESSAGE_ENCODER_H_
#define HOOD_PROXY_TLS_MESSAGE_ENCODER_H_

#include <unordered_map>
#include <vector>

#include "tls_definition.hpp"

namespace hood_proxy {
namespace tls {

class MessageEncoder {
 public:
  enum class ResultType { good, bad };
  static ResultType Encode(const Message& message, std::vector<uint8_t>& buffer,
                           size_t offset);

  static ResultType Truncate(uint8_t* buffer, size_t buffer_size,
                             size_t size_limit, size_t& truncated_size);
};

}  // namespace tls
}  // namespace hood_proxy
#endif  // HOOD_PROXY_TLS_MESSAGE_ENCODER_H_