#include <algorithm>
#include <boost/endian/conversion.hpp>
#include <functional>
#include <limits>
#include <string>
#include <unordered_map>
#include <vector>

#include "logging.hpp"
#include "tls_message_encoder.hpp"

namespace endian = boost::endian;
using std::string;
using std::unordered_map;
using std::vector;

namespace hood_proxy {
namespace tls {

#define SAFE_SET_INT(TO_, FROM_)                                      \
  do {                                                                \
    if (std::numeric_limits<decltype(TO_)>::max() < (FROM_) ||        \
        std::numeric_limits<decltype(TO_)>::min() > (FROM_)) {        \
      return MessageEncoder::ResultType::bad;                         \
    }                                                                 \
    (TO_) = endian::native_to_big(static_cast<decltype(TO_)>(FROM_)); \
  } while (false)

enum class VectorLengthMode { ELEMENT_COUNT, BYTE_SIZE };
template <typename LengthType, typename ValueType, VectorLengthMode Mode>
static MessageEncoder::ResultType EncodeVector(
    const std::vector<ValueType>& source, std::vector<uint8_t>& buffer,
    size_t& offset) {
  static_assert(std::is_arithmetic_v<LengthType>);
  auto count = source.size();
  auto byte_size = count * sizeof(ValueType);
  buffer.resize(offset + sizeof(LengthType) + byte_size);

#pragma pack(push, 1)
  struct {
    LengthType length;
    ValueType values[];
  }* header = reinterpret_cast<decltype(header)>(buffer.data() + offset);
#pragma pack(pop)

  if constexpr (Mode == VectorLengthMode::BYTE_SIZE) {
    SAFE_SET_INT(header->length, byte_size);
  } else {
    SAFE_SET_INT(header->length, count);
  }

  if constexpr (std::is_arithmetic_v<ValueType> && sizeof(ValueType) > 1 &&
                endian::order::native != endian::order::big) {
    for (LengthType i = 0; i < count; i++) {
      header->values[i] = endian::native_to_big(source[i]);
    }
  } else {
    memcpy(header->values, &source[0], byte_size);
  }
  offset = buffer.size();
  return MessageEncoder::ResultType::good;
}

MessageEncoder::ResultType EncodeServerName(
    const extension::ServerName& extension, std::vector<uint8_t>& buffer,
    size_t& offset) {
  buffer.resize(offset + sizeof(protocol::extension::ServerNameList));
  auto list_header =
      reinterpret_cast<protocol::extension::ServerNameList*>(&buffer[offset]);
  offset += sizeof(protocol::extension::ServerNameList);
  auto content_offset = offset;
  if (extension.host_name.length()) {
    buffer.resize(offset + sizeof(protocol::extension::ServerName));
    auto name_header =
        reinterpret_cast<protocol::extension::ServerName*>(&buffer[offset]);
    offset += sizeof(protocol::extension::ServerName);
    name_header->name_type = protocol::extension::ServerNameType::host_name;
    auto name_length = extension.host_name.length();
    SAFE_SET_INT(name_header->length, name_length);
    buffer.insert(buffer.end(), extension.host_name.begin(),
                  extension.host_name.end());
    offset += name_length;

  } else {
    return MessageEncoder::ResultType::bad;
  }
  SAFE_SET_INT(list_header->length, buffer.size() - content_offset);
  return MessageEncoder::ResultType::good;
}
MessageEncoder::ResultType EncodeExtensions(
    const extension::Extensions& extensions, std::vector<uint8_t>& buffer,
    size_t& offset) {
  buffer.resize(offset + sizeof(protocol::extension::Extensions));
  auto section_header =
      reinterpret_cast<protocol::extension::Extensions*>(&buffer[offset]);
  offset += sizeof(protocol::extension::Extensions);
  auto content_offset = offset;
  for (auto& extension : extensions) {
    buffer.resize(offset + sizeof(protocol::extension::Extension));
    auto extension_header =
        reinterpret_cast<protocol::extension::Extension*>(&buffer[offset]);
    offset += sizeof(protocol::extension::Extension);
    auto extension_content_offset = offset;
    if (extension.type == protocol::extension::Type::server_name) {
      auto result = EncodeServerName(
          std::get<extension::ServerName>(extension.content), buffer, offset);
      if (result == MessageEncoder::ResultType::bad) {
        return result;
      }
    } else if (extension.type == protocol::extension::Type::supported_groups) {
      auto result =
          EncodeVector<decltype(protocol::extension::NamedGroupList::length),
                       protocol::extension::NamedGroup,
                       VectorLengthMode::BYTE_SIZE>(
              std::get<extension::NamedGroupList>(extension.content), buffer,
              offset);
      if (result == MessageEncoder::ResultType::bad) {
        return result;
      }
    } else if (extension.type ==
               protocol::extension::Type::supported_versions) {
      auto& versions =
          std::get<extension::SupportedVersions>(extension.content);
      if (versions.size() == 1) {
        buffer.resize(offset + sizeof(protocol::VersionType));
        auto version =
            reinterpret_cast<protocol::VersionType*>(&buffer[offset]);
        *version = endian::native_to_big(versions[0]);
        offset += sizeof(protocol::VersionType);
      } else if (versions.size() > 1) {
        auto result = EncodeVector<
            decltype(protocol::extension::SupportedVersionList::length),
            protocol::VersionType, VectorLengthMode::BYTE_SIZE>(versions,
                                                                buffer, offset);
        if (result == MessageEncoder::ResultType::bad) {
          LOG_DEBUG();
          return result;
        }
      } else {
        LOG_DEBUG();
        return MessageEncoder::ResultType::bad;
      }
    } else {
      auto& content = std::get<std::vector<uint8_t>>(extension.content);
      buffer.insert(buffer.end(), content.begin(), content.end());
      offset = buffer.size();
    }
    extension_header->type = endian::native_to_big(extension.type);
    SAFE_SET_INT(extension_header->length,
                 buffer.size() - extension_content_offset);
  }
  SAFE_SET_INT(section_header->length, buffer.size() - content_offset);
  offset = buffer.size();
  return MessageEncoder::ResultType::good;
}

static MessageEncoder::ResultType EncodeClientHello(
    const handshake::ClientHello& message, std::vector<uint8_t>& buffer,
    size_t& offset) {
  using ResultType = MessageEncoder::ResultType;
  buffer.resize(offset +
                sizeof(protocol::handshake::RawClientHello::FixedLengthHeader));
  auto header =
      reinterpret_cast<protocol::handshake::RawClientHello::FixedLengthHeader*>(
          buffer.data() + offset);
  header->legacy_version = endian::native_to_big(message.legacy_version);
  static_assert(
      sizeof(handshake::ClientHello::random) ==
      sizeof(protocol::handshake::RawClientHello::FixedLengthHeader::random));
  memcpy(header->random, message.random.data(), message.random.size());
  offset += sizeof(protocol::handshake::RawClientHello::FixedLengthHeader);

  auto result = EncodeVector<uint8_t, uint8_t, VectorLengthMode::ELEMENT_COUNT>(
      message.legacy_session_id, buffer, offset);
  if (result == ResultType::bad) {
    return result;
  }
  result = EncodeVector<uint16_t, CipherSuite, VectorLengthMode::BYTE_SIZE>(
      message.cipher_suites, buffer, offset);
  if (result == ResultType::bad) {
    return result;
  }

  result = EncodeVector<uint8_t, uint8_t, VectorLengthMode::ELEMENT_COUNT>(
      message.legacy_compression_methods, buffer, offset);
  if (result == ResultType::bad) {
    return result;
  }
  result = EncodeExtensions(message.extensions, buffer, offset);
  if (result != ResultType::good) {
    return result;
  }
  offset = buffer.size();
  return ResultType::good;
}

static MessageEncoder::ResultType EncodeServerHello(
    const handshake::ServerHello& message, std::vector<uint8_t>& buffer,
    size_t& offset) {
  using ResultType = MessageEncoder::ResultType;
  buffer.resize(offset +
                sizeof(protocol::handshake::RawServerHello::FixedLengthHeader));
  auto header =
      reinterpret_cast<protocol::handshake::RawServerHello::FixedLengthHeader*>(
          buffer.data() + offset);
  header->legacy_version = endian::native_to_big(message.legacy_version);
  static_assert(
      sizeof(handshake::ServerHello::random) ==
      sizeof(protocol::handshake::RawServerHello::FixedLengthHeader::random));
  memcpy(header->random, message.random.data(), message.random.size());
  offset += sizeof(protocol::handshake::RawServerHello::FixedLengthHeader);

  auto result = EncodeVector<uint8_t, uint8_t, VectorLengthMode::ELEMENT_COUNT>(
      message.legacy_session_id_echo, buffer, offset);
  if (result == ResultType::bad) {
    return result;
  }

  {
    buffer.resize(offset + sizeof(protocol::CipherSuite));
    *reinterpret_cast<protocol::CipherSuite*>(&buffer[offset]) =
        boost::endian::native_to_big(message.cipher_suite);
    offset += sizeof(protocol::CipherSuite);
  }
  {
    buffer.resize(offset + 1);
    static_assert(sizeof(message.legacy_compression_method) == 1);

    *reinterpret_cast<int8_t*>(&buffer[offset]) =
        message.legacy_compression_method;
    offset += 1;
  }
  result = EncodeExtensions(message.extensions, buffer, offset);
  if (result != ResultType::good) {
    return result;
  }
  offset = buffer.size();
  return ResultType::good;
}

static MessageEncoder::ResultType EncodeHandshake(
    const handshake::Message& message, std::vector<uint8_t>& buffer,
    size_t& offset) {
  using ResultType = MessageEncoder::ResultType;
  buffer.resize(offset + sizeof(protocol::handshake::RawHandshake));
  auto header = reinterpret_cast<protocol::handshake::RawHandshake*>(
      buffer.data() + offset);
  offset += sizeof(protocol::handshake::RawHandshake);
  auto content_offset = offset;
  header->msg_type = message.type;
  if (message.type == protocol::handshake::Type::client_hello) {
    const auto& client_hello_message =
        std::get<handshake::ClientHello>(message.content);
    auto result = EncodeClientHello(client_hello_message, buffer, offset);
    if (result != MessageEncoder::ResultType::good) {
      return result;
    }
  } else if (message.type == protocol::handshake::Type::server_hello) {
    const auto& server_hello_message =
        std::get<handshake::ServerHello>(message.content);
    auto result = EncodeServerHello(server_hello_message, buffer, offset);
    if (result != MessageEncoder::ResultType::good) {
      return result;
    }
  } else {
    auto& content = std::get<std::vector<uint8_t>>(message.content);
    buffer.insert(buffer.end(), content.begin(), content.end());
    offset = buffer.size();
  }
  /*else if (message.type == protocol::handshake::Type::new_session_ticket) {
  } else if (message.type == protocol::handshake::Type::end_of_early_data) {
  } else if (message.type == protocol::handshake::Type::encrypted_extensions) {
  } else if (message.type == protocol::handshake::Type::certificate) {
  } else if (message.type == protocol::handshake::Type::certificate_request) {
  } else if (message.type == protocol::handshake::Type::certificate_verify) {
  } else if (message.type == protocol::handshake::Type::finished) {
  } else if (message.type == protocol::handshake::Type::key_update) {
  } else if (message.type == protocol::handshake::Type::message_hash) {
  }*/

  SetUint24Value(header->length, buffer.size() - content_offset);
  offset = buffer.size();
  return ResultType::good;
}

MessageEncoder::ResultType MessageEncoder::Encode(const Message& message,
                                                  std::vector<uint8_t>& buffer,
                                                  size_t offset) {
  buffer.reserve(offset + 2048);

  // encode header
  buffer.resize(offset + sizeof(protocol::TLSPlaintext));
  auto tls_header =
      reinterpret_cast<protocol::TLSPlaintext*>(buffer.data() + offset);
  tls_header->type = message.type;
  tls_header->legacy_record_version =
      endian::native_to_big(message.legacy_record_version);
  offset += sizeof(protocol::TLSPlaintext);
  auto content_offset = offset;

  if (message.type == protocol::ContentType::handshake) {
    const auto& handshake_message =
        std::get<handshake::Message>(message.content);
    auto result = EncodeHandshake(handshake_message, buffer, offset);
    if (result != MessageEncoder::ResultType::good) {
      return result;
    }
  }
  SAFE_SET_INT(tls_header->message_length, buffer.size() - content_offset);
  return ResultType::good;
}

MessageEncoder::ResultType MessageEncoder::Truncate(uint8_t* buffer,
                                                    size_t buffer_size,
                                                    size_t size_limit,
                                                    size_t& truncated_size) {
  buffer = buffer;
  buffer_size = buffer_size;
  truncated_size = size_limit;
  return ResultType::good;
}

}  // namespace tls
}  // namespace hood_proxy