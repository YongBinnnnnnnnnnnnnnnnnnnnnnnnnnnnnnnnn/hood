#include <boost/endian/conversion.hpp>
#include <string>
#include <type_traits>
#include <vector>

#include "logging.hpp"
#include "src/tls_definition.hpp"
#include "src/tls_definition_raw.hpp"
#include "tls_message_decoder.hpp"

namespace endian = boost::endian;
using std::string;
using std::vector;

namespace hood_proxy {
namespace tls {

template <typename OutputType>
static inline constexpr MessageDecoder::ResultType Decode(
    auto& save_to,
    std::function<MessageDecoder::ResultType(OutputType&, const uint8_t*,
                                             size_t, size_t, size_t&)>
        decoder,
    const uint8_t* buffer, size_t buffer_size, size_t from_offset,
    size_t& end_offset) {
  OutputType decode_output;
  auto result =
      decoder(decode_output, buffer, buffer_size, from_offset, end_offset);
  if (result == MessageDecoder::ResultType::good) {
    save_to = std::move(decode_output);
  }
  return result;
}

MessageDecoder::ResultType MessageDecoder::DecodeMesssage(Message& message,
                                                          const uint8_t* buffer,
                                                          size_t buffer_size) {
  size_t offset = 0;
  if (buffer_size < offset + sizeof(protocol::TLSPlaintext)) {
    return ResultType::bad;
  }
  // decode header
  auto header = reinterpret_cast<const protocol::TLSPlaintext*>(buffer);
  message.type = header->type;
  message.legacy_record_version =
      endian::big_to_native(header->legacy_record_version);

  auto message_length = boost::endian::big_to_native(header->mesage_length);
  offset = sizeof(protocol::TLSPlaintext);
  if (buffer_size < offset + message_length) {
    return ResultType::bad;
  }
  if (message.type == protocol::ContentType::handshake) {
    return Decode<handshake::Message>(message.content, DecodeHandshake,
                                      header->data, message_length, 0, offset);
  } else if (message.type == protocol::ContentType::handshake) {
  }
  return ResultType::bad;
}

inline MessageDecoder::ResultType MessageDecoder::DecodeHandshake(
    handshake::Message& message, const uint8_t* buffer, size_t buffer_size,
    size_t from_offset, size_t& end_offset) {
  auto header = reinterpret_cast<const protocol::handshake::RawHandshake*>(
      buffer + from_offset);
  message.type = header->msg_type;
  auto message_length = protocol::GetUint24Value(header->length);
  auto offset = from_offset;
  end_offset = offset + message_length;
  if (end_offset > buffer_size) {
    return ResultType::bad;
  }
  if (message.type == protocol::handshake::Type::client_hello) {
    return Decode<handshake::ClientHello>(message.content, DecodeClientHello,
                                          header->data, buffer_size, offset,
                                          offset);
  } else if (message.type == protocol::handshake::Type::server_hello) {
  } else if (message.type == protocol::handshake::Type::new_session_ticket) {
  } else if (message.type == protocol::handshake::Type::end_of_early_data) {
  } else if (message.type == protocol::handshake::Type::encrypted_extensions) {
  } else if (message.type == protocol::handshake::Type::certificate) {
  } else if (message.type == protocol::handshake::Type::certificate_request) {
  } else if (message.type == protocol::handshake::Type::certificate_verify) {
  } else if (message.type == protocol::handshake::Type::finished) {
  } else if (message.type == protocol::handshake::Type::key_update) {
  } else if (message.type == protocol::handshake::Type::message_hash) {
  }
  return ResultType::bad;
}
enum class VectorLengthMode { ELEMENT_COUNT, BYTE_SIZE };
template <typename LengthType, typename ValueType, VectorLengthMode Mode>
static constexpr inline MessageDecoder::ResultType DecodeVector(
    std::vector<ValueType>& output, const uint8_t* buffer, size_t buffer_size,
    size_t from_offset, size_t& end_offset) {
  static_assert(std::is_arithmetic_v<LengthType>);
  auto offset = from_offset;
  if (buffer_size < offset + sizeof(LengthType)) {
    return MessageDecoder::ResultType::bad;
  }
  auto count = *reinterpret_cast<const LengthType*>(buffer + offset);
  size_t size;
  offset += sizeof(LengthType);
  if constexpr (Mode == VectorLengthMode::ELEMENT_COUNT) {
    size = count * sizeof(ValueType);
  } else {
    size = count;
    if (count % sizeof(ValueType)) {
      return MessageDecoder::ResultType::bad;
    }
    count = count / sizeof(ValueType);
  }
  if (buffer_size < offset + count) {
    return MessageDecoder::ResultType::bad;
  }
  output.resize(count);
  if constexpr (std::is_arithmetic_v<ValueType> && sizeof(ValueType) > 1 &&
                endian::order::native != endian::order::big) {
    auto array_pointer = reinterpret_cast<const ValueType*>(buffer + offset);
    for (LengthType i = 0; i < count; i++) {
      output[i] = endian::big_to_native(array_pointer[i]);
    }
  } else {
    memcpy(&output[0], buffer + offset, size);
  }
  end_offset = offset + size;
  return MessageDecoder::ResultType::good;
}

inline MessageDecoder::ResultType MessageDecoder::DecodeClientHello(
    handshake::ClientHello& message, const uint8_t* buffer, size_t buffer_size,
    size_t from_offset, size_t& end_offset) {
  auto offset = from_offset;
  if (buffer_size < offset + sizeof(protocol::handshake::RawClientHello)) {
    return ResultType::bad;
  }
  auto header = reinterpret_cast<const protocol::handshake::RawClientHello*>(
      buffer + from_offset);
  message.legacy_version = boost::endian::big_to_native(header->legacy_version);
  static_assert(sizeof(message.random) == sizeof(header->random));
  memcpy(message.random, header->random, sizeof(message.random));
  offset += offsetof(protocol::handshake::RawClientHello, legacy_session_id);

  auto result = DecodeVector<uint8_t, uint8_t, VectorLengthMode::ELEMENT_COUNT>(
      message.legacy_session_id, buffer, buffer_size, offset, offset);
  if (result == ResultType::bad) {
    return result;
  }

  result = DecodeVector<uint16_t, uint16_t, VectorLengthMode::BYTE_SIZE>(
      message.cipher_suites, buffer, buffer_size, offset, offset);
  if (result == ResultType::bad) {
    return result;
  }

  result = DecodeVector<uint8_t, uint8_t, VectorLengthMode::ELEMENT_COUNT>(
      message.legacy_compression_methods, buffer, buffer_size, offset, offset);
  if (result == ResultType::bad) {
    return result;
  }
  return Decode<Extensions>(message.extensions, DecodeExtensions, buffer,
                            buffer_size, offset, offset);
}

inline MessageDecoder::ResultType MessageDecoder::DecodeExtensions(
    extension::Extensions& output, const uint8_t* buffer, size_t buffer_size,
    size_t from_offset, bool follow_offset_label, size_t& max_offset) {
  // TODO
  return ResultType::bad;
}

}  // namespace tls
}  // namespace hood_proxy