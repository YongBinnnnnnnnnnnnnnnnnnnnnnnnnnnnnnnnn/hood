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

enum class VectorLengthMode { ELEMENT_COUNT, BYTE_SIZE };
template <typename LengthType, typename ValueType, VectorLengthMode Mode>
static constexpr inline MessageDecoder::ResultType DecodeVector(
    std::vector<ValueType>& output, const uint8_t* buffer, size_t buffer_size,
    size_t from_offset, size_t& end_offset) {
  static_assert(std::is_arithmetic_v<LengthType>);
  auto offset = from_offset;
  if (buffer_size < offset + sizeof(LengthType)) {
    LOG_DEBUG();
    return MessageDecoder::ResultType::bad;
  }
  auto count = endian::big_to_native(
      *reinterpret_cast<const LengthType*>(buffer + offset));
  size_t size;
  offset += sizeof(LengthType);
  if constexpr (Mode == VectorLengthMode::ELEMENT_COUNT) {
    size = count * sizeof(ValueType);
  } else {
    size = count;
    if (count % sizeof(ValueType)) {
      LOG_DEBUG();
      return MessageDecoder::ResultType::bad;
    }
    count = count / sizeof(ValueType);
  }
  if (buffer_size < offset + size) {
    LOG_DEBUG();
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

MessageDecoder::ResultType MessageDecoder::DecodeMesssage(Message& message,
                                                          const uint8_t* buffer,
                                                          size_t buffer_size,
                                                          size_t& end_offset) {
  size_t offset = 0;
  if (buffer_size < offset + sizeof(protocol::TLSPlaintext)) {
    LOG_DEBUG();
    return ResultType::bad;
  }
  // decode header
  auto header = reinterpret_cast<const protocol::TLSPlaintext*>(buffer);
  message.type = header->type;
  message.legacy_record_version =
      endian::big_to_native(header->legacy_record_version);

  auto message_length = boost::endian::big_to_native(header->message_length);
  offset = sizeof(protocol::TLSPlaintext);
  if (buffer_size < offset + message_length) {
    LOG_DEBUG();
    return ResultType::bad;
  }
  if (message.type == protocol::ContentType::handshake) {
    auto& handshake_messages = message.content.emplace<handshake::Messages>();
    return DecodeHandshake(handshake_messages, buffer, offset + message_length,
                           offset, end_offset);
  } else {
    message.content.emplace<std::vector<uint8_t>>(
        &buffer[offset], &buffer[offset + message_length]);
    end_offset = offset + message_length;
  }
  return ResultType::good;
}

inline MessageDecoder::ResultType MessageDecoder::DecodeServerHello(
    handshake::ServerHello& message, const uint8_t* buffer, size_t buffer_size,
    size_t from_offset, size_t& end_offset) {
  auto offset = from_offset;
  if (buffer_size <
      offset + sizeof(protocol::handshake::RawServerHello::FixedLengthHeader)) {
    LOG_DEBUG();
    return ResultType::bad;
  }
  auto header = reinterpret_cast<
      const protocol::handshake::RawServerHello::FixedLengthHeader*>(
      buffer + from_offset);

  message.legacy_version = boost::endian::big_to_native(header->legacy_version);
  static_assert(sizeof(message.random) == sizeof(header->random));
  memcpy(message.random.data(), header->random, sizeof(message.random));

  using HelloRetryRequestSpecialValue =
      protocol::handshake::HelloRetryRequestSpecialValue;
  if (memcmp(header->random, HelloRetryRequestSpecialValue::head.data(),
             HelloRetryRequestSpecialValue::head.size()) == 0) {
    auto tail = &header->random[HelloRetryRequestSpecialValue::head.size()];
    if (memcmp(tail, HelloRetryRequestSpecialValue::tail_1_3.data(),
               HelloRetryRequestSpecialValue::tail_1_3.size()) == 0) {
      message.hello_retry_type = handshake::HelloRetryType::FOR_TLS_1_3;
    } else if (memcmp(tail, HelloRetryRequestSpecialValue::tail_1_2.data(),
                      HelloRetryRequestSpecialValue::tail_1_2.size()) == 0) {
      message.hello_retry_type = handshake::HelloRetryType::FOR_TLS_1_2;
    } else if (memcmp(tail, HelloRetryRequestSpecialValue::tail_1_1.data(),
                      HelloRetryRequestSpecialValue::tail_1_1.size()) == 0) {
      message.hello_retry_type = handshake::HelloRetryType::FOR_TLS_1_1;
    } else {
      message.hello_retry_type = handshake::HelloRetryType::FOR_UNKNOWN;
    }
  } else {
    message.hello_retry_type = handshake::HelloRetryType::NOT_RETRY;
  }
  offset += sizeof(protocol::handshake::RawServerHello::FixedLengthHeader);

  auto result = DecodeVector<uint8_t, uint8_t, VectorLengthMode::ELEMENT_COUNT>(
      message.legacy_session_id_echo, buffer, buffer_size, offset, offset);
  if (result == ResultType::bad) {
    return result;
  }
  {
    message.cipher_suite = boost::endian::big_to_native(
        *reinterpret_cast<const protocol::CipherSuite*>(buffer + offset));
    offset += sizeof(protocol::CipherSuite);
  }
  {
    static_assert(sizeof(message.legacy_compression_method) == 1);
    message.legacy_compression_method =
        *reinterpret_cast<const int8_t*>(buffer + offset);
    offset += 1;
  }
  return DecodeExtensions(message.extensions, buffer, buffer_size, offset,
                          end_offset);
}

inline MessageDecoder::ResultType MessageDecoder::DecodeHandshake(
    handshake::Messages& the_messages, const uint8_t* buffer,
    size_t buffer_size, size_t from_offset, size_t& end_offset) {
  while (from_offset < buffer_size) {
    auto header = reinterpret_cast<const protocol::handshake::RawHandshake*>(
        buffer + from_offset);
    auto& message = the_messages.emplace_back();
    message.type = header->msg_type;
    auto message_length = protocol::GetUint24Value(header->length);
    auto offset = from_offset + sizeof(protocol::handshake::RawHandshake);
    end_offset = offset + message_length;
    if (end_offset > buffer_size) {
      LOG_DEBUG();
      return ResultType::bad;
    }
    if (message.type == protocol::handshake::Type::client_hello) {
      auto result =
          DecodeClientHello(message.content.emplace<handshake::ClientHello>(),
                            buffer, buffer_size, offset, offset);
      if (result == ResultType::bad) {
        LOG_DEBUG();
        return result;
      }
    } else if (message.type == protocol::handshake::Type::server_hello) {
      auto result =
          DecodeServerHello(message.content.emplace<handshake::ServerHello>(),
                            buffer, buffer_size, offset, offset);
      if (result == ResultType::bad) {
        LOG_DEBUG();
        return result;
      }
    } else {
      message.content.emplace<std::vector<uint8_t>>(
          header->data, header->data + message_length);
    } /* TODO
    else if (message.type == protocol::handshake::Type::certificate) {
    }
    else if (message.type == protocol::handshake::Type::certificate_request) {
    }
    else if (message.type == protocol::handshake::Type::certificate_verify) {
    } */
    from_offset = end_offset;
  }
  return ResultType::good;
}

inline MessageDecoder::ResultType MessageDecoder::DecodeClientHello(
    handshake::ClientHello& message, const uint8_t* buffer, size_t buffer_size,
    size_t from_offset, size_t& end_offset) {
  auto offset = from_offset;
  if (buffer_size <
      offset + sizeof(protocol::handshake::RawClientHello::FixedLengthHeader)) {
    LOG_DEBUG();
    return ResultType::bad;
  }
  auto header = reinterpret_cast<
      const protocol::handshake::RawClientHello::FixedLengthHeader*>(
      buffer + from_offset);
  message.legacy_version = boost::endian::big_to_native(header->legacy_version);
  static_assert(sizeof(message.random) == sizeof(header->random));
  memcpy(message.random.data(), header->random, sizeof(message.random));
  offset += sizeof(protocol::handshake::RawClientHello::FixedLengthHeader);

  auto result = DecodeVector<uint8_t, uint8_t, VectorLengthMode::ELEMENT_COUNT>(
      message.legacy_session_id, buffer, buffer_size, offset, offset);
  if (result == ResultType::bad) {
    return result;
  }

  result = DecodeVector<uint16_t, CipherSuite, VectorLengthMode::BYTE_SIZE>(
      message.cipher_suites, buffer, buffer_size, offset, offset);
  if (result == ResultType::bad) {
    return result;
  }

  result = DecodeVector<uint8_t, uint8_t, VectorLengthMode::ELEMENT_COUNT>(
      message.legacy_compression_methods, buffer, buffer_size, offset, offset);
  if (result == ResultType::bad) {
    return result;
  }
  return DecodeExtensions(message.extensions, buffer, buffer_size, offset,
                          end_offset);
}

inline MessageDecoder::ResultType MessageDecoder::DecodeExtensions(
    extension::Extensions& output, const uint8_t* buffer, size_t buffer_size,
    size_t from_offset, size_t& end_offset) {
  // TODO
  auto offset = from_offset;
  if (buffer_size < offset + sizeof(protocol::extension::Extensions)) {
    LOG_DEBUG();
    return ResultType::bad;
  }
  auto header = reinterpret_cast<const protocol::extension::Extensions*>(
      buffer + from_offset);
  offset += sizeof(protocol::extension::Extensions);
  end_offset = offset + endian::big_to_native(header->length);
  if (end_offset > buffer_size) {
    LOG_DEBUG();
    return ResultType::bad;
  }
  while (offset < end_offset) {
    if (end_offset < offset + sizeof(protocol::extension::Extension)) {
      LOG_DEBUG();
      return ResultType::bad;
    }
    auto header = reinterpret_cast<const protocol::extension::Extension*>(
        buffer + offset);
    auto& extension = output.emplace_back();
    extension.type = endian::big_to_native(header->type);
    auto length = endian::big_to_native(header->length);
    offset += sizeof(protocol::extension::Extension);
    auto end_of_extension = length + offset;
    if (end_offset < end_of_extension) {
      LOG_DEBUG();
      return ResultType::bad;
    }
    if (extension.type == protocol::extension::Type::server_name) {
      if (length == 0) {
        extension.content.emplace<extension::ServerName>();
        return ResultType::good;
      }
      if (end_of_extension < offset + sizeof(protocol::extension::ServerName)) {
        LOG_DEBUG();
        return ResultType::bad;
      }
      offset += sizeof(protocol::extension::Type::server_name);
      auto server_name_header =
          reinterpret_cast<const protocol::extension::ServerName*>(buffer +
                                                                   offset);
      auto type = server_name_header->name_type;
      auto server_name_length =
          endian::big_to_native(server_name_header->length);
      offset += sizeof(protocol::extension::ServerName);
      if (end_of_extension != offset + server_name_length) {
        LOG_DEBUG();
        return ResultType::bad;
      }
      if (type != protocol::extension::ServerNameType::host_name) {
        LOG_DEBUG();
        return ResultType::bad;
      }
      auto& server_name = extension.content.emplace<extension::ServerName>();
      server_name.host_name.assign(server_name_header->name,
                                   server_name_length);
    } else if (extension.type == protocol::extension::Type::supported_groups) {
      auto& groups = extension.content.emplace<extension::NamedGroupList>();
      auto result =
          DecodeVector<typeof(protocol::extension::NamedGroupList::length),
                       protocol::extension::NamedGroup,
                       VectorLengthMode::BYTE_SIZE>(groups, buffer, buffer_size,
                                                    offset, offset);
      if (result == ResultType::bad || offset != end_of_extension) {
        return result;
      }
    } else if (extension.type ==
               protocol::extension::Type::supported_versions) {
      auto& versions =
          extension.content.emplace<extension::SupportedVersions>();
      if (length == sizeof(protocol::VersionType)) {
        auto version =
            reinterpret_cast<const protocol::VersionType*>(&buffer[offset]);
        versions.emplace_back(endian::big_to_native(*version));
        offset += length;
      } else if (length > sizeof(protocol::VersionType)) {
        auto result = DecodeVector<
            typeof(protocol::extension::SupportedVersionList::length),
            protocol::VersionType, VectorLengthMode::BYTE_SIZE>(
            versions, buffer, buffer_size, offset, offset);
        if (result == ResultType::bad || offset != end_of_extension) {
          return result;
        }
      } else {
        LOG_DEBUG();
        return ResultType::bad;
      }
    } else {
      extension.content.emplace<vector<uint8_t>>(buffer + offset,
                                                 buffer + end_of_extension);
    }
    offset = end_of_extension;
  }
  return ResultType::good;
}

}  // namespace tls
}  // namespace hood_proxy