#include <algorithm>
#include <boost/endian/conversion.hpp>
#include <functional>
#include <limits>
#include <string>
#include <unordered_map>
#include <vector>

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

MessageEncoder::ResultType EncodeExtension(const Extension& extension,
                                           std::vector<uint8_t>& buffer,
                                           size_t& offset) {
  return MessageEncoder::ResultType::bad;
}

MessageEncoder::ResultType EncodeClientHello(
    const handshake::ClientHello& message, std::vector<uint8_t>& buffer,
    size_t& offset) {
  buffer.resize(offset +
                sizeof(protocol::handshake::RawClientHello::FixedLengthHead));
  auto header = reinterpret_cast<protocol::handshake::FixedLengthHead*>(
      buffer.data() + offset);
  header->legacy_version = endian::native_to_big(message.legacy_version);
  for (const auto& extension : message.extensions) {
    auto result = EncodeExtension(extension, buffer, offset);
    if (result != MessageEncoder::ResultType::good) {
      return result;
    }
  }
}

MessageEncoder::ResultType MessageEncoder::Encode(const Message& message,
                                                  std::vector<uint8_t>& buffer,
                                                  size_t offset) {
  buffer.reserve(offset + 2048);
  auto start_offset = offset;

  // encode header
  buffer.resize(offset + sizeof(protocol::TLSPlaintext));
  auto tls_header =
      reinterpret_cast<protocol::TLSPlaintext*>(buffer.data() + offset);
  tls_header->type = message.type;
  tls_header->legacy_record_version =
      endian::native_to_big(message.legacy_record_version);
  offset += sizeof(protocol::TLSPlaintext);

  if (message.type == protocol::handshake::Type::client_hello) {
    const auto& client_hello_message =
        std::get<handshake::ClientHello>(message.content);
    auto result = EncodeClientHello(client_hello_message, buffer, offset);
    if (result != MessageEncoder::ResultType::good) {
      return result;
    }

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
  SAFE_SET_INT(tls_header->message_length, buffer.size() - start_offset);
  return ResultType::good;
}

MessageEncoder::ResultType MessageEncoder::Truncate(uint8_t* buffer,
                                                    size_t buffer_size,
                                                    size_t size_limit,
                                                    size_t& truncated_size) {
  if (size_limit < sizeof(RawHeader)) {
    return ResultType::bad;
  }
  MessageView message;
  size_t new_size;
  {
    // decode to message_view to get offsets
    MessageDecoder decoder;
    auto decode_result =
        decoder.ViewData(message, buffer, buffer_size, new_size);
    if (decode_result != MessageDecoder::ResultType::good) {
      return ResultType::bad;
    }
  }

  if (message.size < size_limit) {
    return ResultType::good;
  }

  // default udp payload size (65507) is close to maxium message size (65535),
  // so start lookup from end to begining
  auto remove_sections = [&new_size, size_limit](vector<size_t>& offsets,
                                                 uint16_t& section_count) {
    while (section_count) {
      section_count--;
      new_size = offsets.back();
      offsets.pop_back();
      if (new_size < size_limit) {
        return;
      }
    }
  };

  remove_sections(message.resource_record_offsets, message.additional_count);
  if (new_size >= size_limit) {
    remove_sections(message.resource_record_offsets, message.authority_count);
  }
  if (new_size >= size_limit) {
    remove_sections(message.resource_record_offsets, message.answer_count);
  }
  {
    uint16_t question_count;
    SAFE_SET_INT(question_count, message.question_offsets.size());
    if (new_size >= size_limit) {
      remove_sections(message.question_offsets, question_count);
    }
  }

  if (new_size >= size_limit) {
    assert(false);
    // impossible unless MessageDecoder is wrong
    return ResultType::bad;
  }

  auto header = reinterpret_cast<RawHeader*>(buffer);
  WRITE_FLAG(header->FLAGS, TC, 1);
  SAFE_SET_INT(header->QDCOUNT, message.question_offsets.size());
  SAFE_SET_INT(header->ANCOUNT, message.answer_count);
  SAFE_SET_INT(header->NSCOUNT, message.authority_count);
  SAFE_SET_INT(header->ARCOUNT, message.additional_count);

  truncated_size = new_size;
  return ResultType::good;
}

MessageEncoder::ResultType MessageEncoder::RewriteIDToTcpMessage(
    uint8_t* buffer, size_t buffer_size, int16_t id) {
  constexpr auto write_offset =
      offsetof(dns::RawTcpMessage, message) + offsetof(dns::RawHeader, ID);
  constexpr auto write_size = sizeof(dns::RawHeader::ID);
  if (buffer_size < write_size + write_size) {
    return ResultType::bad;
  }
  *reinterpret_cast<decltype(dns::RawHeader::ID)*>(buffer + write_offset) =
      boost::endian::native_to_big(id);
  return ResultType::good;
}

MessageEncoder::ResultType
MessageEncoder::EncodeEDNS0ClientSubnetResoureceRecord(
    std::vector<uint8_t>& buffer, uint16_t udp_payload_size,
    EDNSOption::ClientSubnet& options, const uint8_t* address) {
  auto unaligned_length = options.source_prefix_length % 8;
  auto address_memcpy_length = options.source_prefix_length / 8;
  auto address_length = address_memcpy_length;
  if (unaligned_length) {
    address_length++;
  }

  buffer.resize(sizeof(EDNS0ResourceRecord) + sizeof(EDNSOption) +
                sizeof(EDNSOption::ClientSubnet) + address_length);
  auto resource_record = reinterpret_cast<EDNS0ResourceRecord*>(buffer.data());
  memset(resource_record, 0, sizeof(EDNS0ResourceRecord));
  resource_record->type =
      endian::native_to_big(static_cast<uint16_t>(TYPE::OPT));
  resource_record->udp_payload_size = endian::native_to_big(udp_payload_size);
  resource_record->rdata_length = endian::native_to_big(static_cast<uint16_t>(
      sizeof(EDNSOption) + sizeof(EDNSOption::ClientSubnet) + address_length));
  auto option = reinterpret_cast<EDNSOption*>(resource_record->rdata);
  option->code = endian::native_to_big(
      static_cast<uint16_t>(EDNSOption::CodeType::CLIENT_SUBNET));
  option->length = endian::native_to_big(
      static_cast<uint16_t>(sizeof(EDNSOption::ClientSubnet) + address_length));
  auto client_subnet =
      reinterpret_cast<EDNSOption::ClientSubnet*>(option->data);
  client_subnet->family = endian::native_to_big(options.family);
  client_subnet->source_prefix_length = options.source_prefix_length;
  client_subnet->scope_prefix_length = options.scope_prefix_length;
  memcpy(client_subnet->address, address, address_memcpy_length);
  if (unaligned_length) {
    client_subnet->address[address_memcpy_length] =
        0b10000000 | address[address_memcpy_length];
    while (unaligned_length > 1) {
      client_subnet->address[address_memcpy_length] |=
          (1 << (8 - unaligned_length)) | address[address_memcpy_length];
      unaligned_length--;
    }
  }
  return ResultType::good;
}

MessageEncoder::ResultType
MessageEncoder::AppendAdditionalResourceRecordToRawTcpMessage(
    std::vector<uint8_t>& message_buffer, const uint8_t* raw_resource_record,
    uint16_t raw_resource_record_length) {
  auto tcp_header = reinterpret_cast<RawTcpMessage*>(message_buffer.data());
  {
    auto message_length = endian::big_to_native(tcp_header->message_length) +
                          raw_resource_record_length;
    SAFE_SET_INT(tcp_header->message_length, message_length);
  }

  auto header = reinterpret_cast<RawHeader*>(tcp_header->message);
  {
    auto arcount = endian::big_to_native(header->ARCOUNT) + 1;
    SAFE_SET_INT(header->ARCOUNT, arcount);
  }
  message_buffer.insert(message_buffer.end(), raw_resource_record,
                        raw_resource_record + raw_resource_record_length);
  return ResultType::good;
}

inline bool EncodeName(MessageEncoderContext& context, const string& name) {
  string::size_type end_offset;
  string::size_type begin_offset = 0;

  while (begin_offset < name.size()) {
    auto i = context.encoded_labels.find(&name[begin_offset]);
    if (i != context.encoded_labels.end()) {
      context.buffer.resize(context.offset + 2);
      auto label =
          reinterpret_cast<RawLabel*>(context.buffer.data() + context.offset);
      label->offset_type.high_part_with_flag = (i->second >> 8) & 0xFF;
      label->offset_type.low_part = i->second & 0xFF;
      label->flag |= RawLabel::Flag::OFFSET;
      context.offset += 2;
      return true;
    }

    end_offset = name.find('.', begin_offset);
    if (end_offset == string::npos) {
      end_offset = name.size();
    }
    auto label_length = end_offset - begin_offset;
    if (label_length > std::numeric_limits<uint8_t>::max()) {
      return false;
    }
    context.buffer.push_back(label_length);
    context.buffer.insert(context.buffer.end(), &name[begin_offset],
                          &name[end_offset]);
    if (label_length > 1) {
      context.encoded_labels[&name[begin_offset]] = context.offset;
    }
    context.offset += 1 + label_length;
    begin_offset = end_offset + 1;
  }
  context.buffer.push_back(0);
  context.offset += 1;
  return true;
}

inline MessageEncoder::ResultType EncodeResourceRecord(
    MessageEncoderContext& context, const ResourceRecord& record) {
  if (!EncodeName(context, record.name)) {
    return MessageEncoder::ResultType::bad;
  }

  auto raw_record = reinterpret_cast<RawResourceRecord*>(
      context.buffer.data() + context.offset - sizeof(RawResourceRecord::NAME));

  raw_record->TYPE = endian::native_to_big(record.type);
  raw_record->CLASS = endian::native_to_big(record.normal_type.the_class);
  raw_record->TTL = endian::native_to_big(record.normal_type.ttl);
  SAFE_SET_INT(raw_record->RDLENGTH, record.rdata.size());
  context.buffer.insert(context.buffer.end(), record.rdata.begin(),
                        record.rdata.end());
  return MessageEncoder::ResultType::good;
}

}  // namespace tls
}  // namespace hood_proxy