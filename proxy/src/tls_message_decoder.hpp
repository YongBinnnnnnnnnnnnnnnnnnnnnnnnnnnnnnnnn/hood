#ifndef HOOD_PROXY_TLS_MESSAGE_DECODER_H_
#define HOOD_PROXY_TLS_MESSAGE_DECODER_H_

#include <boost/endian/conversion.hpp>
#include <string>
#include <vector>

#include "tls_definition.hpp"

namespace hood_proxy {
namespace tls {

class MessageDecoder {
 public:
  enum class ResultType { good, bad, indeterminate };
  static ResultType DecodeMesssage(Message& message, const uint8_t* buffer,
                                   size_t buffer_size, size_t& end_offset);

 private:
  enum class FieldType {
    NAME,
    AFTER_NAME,
    RDATA,
  } waiting_field_ = FieldType::NAME;

  static ResultType DecodeHandshake(handshake::Messages& the_messages,
                                    const uint8_t* buffer, size_t buffer_size,
                                    size_t from_offset, size_t& end_offset);
  static ResultType DecodeClientHello(handshake::ClientHello& message,
                                      const uint8_t* buffer, size_t buffer_size,
                                      size_t from_offset, size_t& end_offset);
  static ResultType DecodeExtensions(extension::Extensions& output,
                                     const uint8_t* buffer, size_t buffer_size,
                                     size_t from_offset, size_t& end_offset);
  static ResultType DecodeServerHello(handshake::ServerHello& message,
                                      const uint8_t* buffer, size_t buffer_size,
                                      size_t from_offset, size_t& end_offset);
};

}  // namespace tls
}  // namespace hood_proxy
#endif  // HOOD_PROXY_TLS_MESSAGE_DECODER_H_