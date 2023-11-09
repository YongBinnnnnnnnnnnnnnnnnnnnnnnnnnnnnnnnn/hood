#include <boost/endian/conversion.hpp>
#include <chrono>
#include <iostream>
#include <vector>

#include "logging.hpp"
#include "tls_context.hpp"
#include "tls_message_decoder.hpp"

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
namespace tls {

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

void Context::HandleUserMessage(TlsMessageReader::Reason reason,
                                const uint8_t* data, uint16_t data_size) {
  if (reason != TlsMessageReader::Reason::NEW_MESSAGE) {
    LOG_TRACE("ignore reason:" << static_cast<int>(reason));
    return;
  }
  if (!data || !data_size) {
    LOG_ERROR("empty message!");
    return;
  }
  using ResultType = MessageDecoder::ResultType;
  LOG_TRACE("received query");
  size_t decoded_message_size = 0;
  auto write_task_pointer = std::make_unique<WriteTask>();
  auto& message = write_task_pointer->message;

  auto decode_result = MessageDecoder::DecodeMesssage(message, data, data_size,
                                                      decoded_message_size);
  if (decode_result != ResultType::good) {
    LOG_ERROR("decode failed!");
    return;
  }

  if (message.type == protocol::ContentType::handshake) {
    LOG_INFO("Received handshake");
    if (message.legacy_record_version < protocol::Version::TLS_1_0) {
      LOG_INFO("Discard connection due to protocol version"
               << message.legacy_record_version);
      // TODO
      return;
    }
    auto handshake_message = std::get<handshake::Message>(message.content);
    if (handshake_message.type == protocol::handshake::Type::client_hello) {
      if (status_ != Status::CLIENT_CONNECTED) {
        LOG_INFO("Discard connection due to client hello timing");
        // TODO
        return;
      }
      auto client_hello_message =
          std::get<handshake::ClientHello>(handshake_message.content);

      std::string host_name(
          extension::FindHostName(client_hello_message.extensions));
      if (host_name.length() == 0) {
        LOG_INFO("Discard connection due to find no host name");
        // TODO
        return;
      }

      LOG_INFO("" << host_name);
    }
  } else if (message.type == protocol::ContentType::application_data) {
    // TODO
  }
}

void Context::DoWrite() {
  if (writing_) {
    return;
  }
  if (write_task_queue_.empty()) {
    return;
  }
  writing_ = true;
  auto task = std::move(write_task_queue_.front());
  write_task_queue_.pop();

  auto write_data = task->raw_message.data();
  auto write_size = task->raw_message.size();
  auto to_client = task->to_client;

  auto handler = [this, _ = std::move(task), __ = shared_from_this()](
                     error_code error, size_t) {
    if (error) {
      LOG_ERROR(<< error.message());
    }
    writing_ = false;
    DoWrite();
  };
  if (to_client) {
    if (std::holds_alternative<tcp::socket>(client_socket_)) {
      auto& socket = std::get<tcp::socket>(client_socket_);
      if (!socket.is_open()) {
        return;
      }
      async_write(socket, boost::asio::buffer(write_data, write_size),
                  std::move(handler));
    }
  }
}

Context::~Context() {
  client_message_reader_.Stop();
  server_message_reader_.Stop();
}

}  // namespace tls
}  // namespace hood_proxy