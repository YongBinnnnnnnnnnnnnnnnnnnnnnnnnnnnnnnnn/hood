#include <boost/endian/conversion.hpp>
#include <chrono>
#include <iostream>
#include <vector>

#include "ssl_context.hpp"

namespace endian = boost::endian;
using boost::asio::async_write;
using boost::asio::ip::tcp;
using boost::asio::ip::udp;
using boost::system::error_code;
using std::cout;
using std::endl;
using std::string;
using std::vector;
using std::chrono::milliseconds;

namespace hood_proxy {
namespace ssl {

void Context::Stop() {
  client_message_reader_.Stop();
  server_message_reader_.Stop();
  if (std::holds_alternative<boost::asio::ip::tcp::socket>(client_socket_)) {
    std::get<tcp::socket>(client_socket_).close();
  }
  if (std::holds_alternative<boost::asio::ip::tcp::socket>(server_socket_)) {
    std::get<tcp::socket>(server_socket_).close();
  }
}

void Context::HandleUserMessage(
    TlsMessageReader::Reason reason, const uint8_t* data, uint16_t data_size,
    const boost::asio::ip::udp::endpoint* udp_endpoint) {
  if (reason != TlsMessageReader::Reason::NEW_MESSAGE) {
    LOG_TRACE("ignore reason:" << static_cast<int>(reason));
    return;
  }
  if (!data || !data_size) {
    LOG_ERROR("empty message!");
    return;
  }
  using ResultType = MessageDecoder::ResultType;
  static auto query_timeout_ = std::chrono::milliseconds(
      Configuration::get("query-timeout").as<uint32_t>());
  uint16_t message_length = data_size;
  uint16_t message_offset = 0;
  LOG_TRACE("tcp query");
  message_offset = offsetof(dns::RawTcpMessage, message);
  message_length -= offsetof(dns::RawTcpMessage, message);
  auto decode_result = dns::MessageDecoder::DecodeCompleteMesssage(
      query->query, data + message_offset, message_length);
  if (decode_result != ResultType::good) {
    LOG_ERROR("decode failed!");
    return;
  }

  // store message as dns::RawTcpMessage
  query->raw_message.resize(offsetof(dns::RawTcpMessage, message) +
                            message_length);
  auto tcp_message =
      reinterpret_cast<dns::RawTcpMessage*>(query->raw_message.data());
  tcp_message->message_length = endian::native_to_big(message_length);
  memcpy(tcp_message->message, data + message_offset, message_length);

  query->ExpiresAfter(query_timeout_);
  Resolver::Resolve(std::move(query),
                    std::bind(&Context::HandleQueryResult, shared_from_this(),
                              std::placeholders::_1, std::placeholders::_2));
}

void Context::DoWrite() {
  if (writing_) {
    return;
  }
  if (reply_queue_.empty()) {
    return;
  }
  writing_ = true;
  auto query = std::move(reply_queue_.front());
  reply_queue_.pop();

  auto& endpoint = query->endpoint;
  auto write_data = query->raw_message.data();
  auto write_size = query->raw_message.size();

  auto handler = [this, _ = std::move(query), __ = shared_from_this()](
                     error_code error, size_t) {
    if (error) {
      LOG_ERROR(<< error.message());
    }
    writing_ = false;
    DoWrite();
  };

  if (std::holds_alternative<udp::socket>(socket_)) {
    auto& socket = std::get<udp::socket>(socket_);
    if (!socket.is_open()) {
      return;
    }
    write_data += offsetof(dns::RawTcpMessage, message);
    write_size -= offsetof(dns::RawTcpMessage, message);
    socket.async_send_to(boost::asio::buffer(write_data, write_size),
                         std::get<udp::endpoint>(endpoint), std::move(handler));
  } else {
    auto& socket = std::get<tcp::socket>(socket_);
    if (!socket.is_open()) {
      return;
    }
    async_write(socket, boost::asio::buffer(write_data, write_size),
                std::move(handler));
  }
}

Context::~Context() { message_reader_.Stop(); }
}  // namespace ssl
}  // namespace hood_proxy