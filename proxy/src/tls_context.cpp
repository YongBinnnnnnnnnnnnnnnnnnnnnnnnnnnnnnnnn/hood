#include <boost/algorithm/string/predicate.hpp>
#include <boost/endian/conversion.hpp>
#include <chrono>
#include <iostream>
#include <vector>

#include "logging.hpp"
#include "network.hpp"
#include "tls_context.hpp"
#include "tls_message_decoder.hpp"
#include "tls_message_encoder.hpp"

namespace endian = boost::endian;
using boost::algorithm::ends_with;
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
  LOG_TRACE("received query");
  size_t decoded_message_size = 0;
  auto write_task_pointer = std::make_unique<WriteTask>();
  write_task_pointer->to_client = false;
  auto& message = write_task_pointer->message;

  auto decode_result = MessageDecoder::DecodeMesssage(message, data, data_size,
                                                      decoded_message_size);
  if (decode_result != MessageDecoder::ResultType::good) {
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

      extension::FindHostName(host_name_, client_hello_message.extensions);
      if (host_name_.length() == 0) {
        LOG_INFO("Discard connection due to find no host name");
        // TODO
        return;
      }

      auto encode_result =
          MessageEncoder::Encode(message, write_task_pointer->raw_message, 0);
      if (encode_result != decltype(encode_result)::good) {
        LOG_ERROR("encode failed!");
        return;
      }
      write_task_queue_.emplace(std::move(write_task_pointer));
      DoResolveConnect(host_name_);
      return;
    }
  }
  auto& write_buffer = write_task_pointer->raw_message;
  write_buffer.resize(0);
  write_buffer.insert(write_buffer.end(), data, data + data_size);
  write_task_queue_.emplace(std::move(write_task_pointer));
  DoWrite();
}

void Context::DoResolveConnect(const std::string& host_name) {
  LOG_INFO("Resolving " << host_name);
  {
    tcp::resolver::query name_query(host_name, "https");

    resolver_.async_resolve(
        name_query, [this, _ = shared_from_this(), host_name](
                        const boost::system::error_code& error,
                        tcp::resolver::results_type results) {
          if (error) {
            LOG_INFO("Discard connection due to error while resolving "
                     << host_name << " " << error.message());
            return;
          }
          typeof(results) end;
          for (; results != end; results++) {
            host_endpoints_.emplace_back(results->endpoint());
          }
          if (host_endpoints_.size() == 0) {
            LOG_INFO("Discard connection due to unable to resolve "
                     << host_name);
            // TODO
            return;
          }
          network::RemoveV6AndLocalEndpoints(host_endpoints_);
          if (host_endpoints_.size() == 0) {
            if (!ends_with(host_name, Configuration::proxy_domain)) {
              DoResolveConnect(host_name + Configuration::proxy_domain.data());
              return;
            }
            LOG_INFO("Discard connection to "
                     << host_name << " because endpoints are not acceptable");
            // TODO
            return;
          }
          DoConnectHost();
        });
  }
}

void Context::HandleServerMessage(TlsMessageReader::Reason reason,
                                  const uint8_t* data, uint16_t data_size) {
  if (reason != TlsMessageReader::Reason::NEW_MESSAGE) {
    LOG_TRACE("ignore reason:" << static_cast<int>(reason));
    return;
  }
  if (!data || !data_size) {
    LOG_ERROR("empty message!");
    return;
  }
  LOG_TRACE("received query");

  size_t decoded_message_size = 0;
  auto write_task_pointer = std::make_unique<WriteTask>();
  write_task_pointer->to_client = true;
  auto& message = write_task_pointer->message;

  auto decode_result = MessageDecoder::DecodeMesssage(message, data, data_size,
                                                      decoded_message_size);
  if (decode_result != MessageDecoder::ResultType::good) {
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
    if (handshake_message.type == protocol::handshake::Type::server_hello) {
      if (status_ != Status::CLIENT_HELLO_FORWARDED) {
        LOG_INFO("Discard connection due to server hello timing");
        // TODO
        return;
      }
      auto server_hello_message =
          std::get<handshake::ServerHello>(handshake_message.content);

      auto encode_result =
          MessageEncoder::Encode(message, write_task_pointer->raw_message, 0);
      if (encode_result != decltype(encode_result)::good) {
        LOG_ERROR("encode failed!");
        return;
      }
      write_task_queue_.emplace(std::move(write_task_pointer));
      DoWrite();
      return;
    }
  }
  auto& write_buffer = write_task_pointer->raw_message;
  write_buffer.resize(0);
  write_buffer.insert(write_buffer.end(), data, data + data_size);
  write_task_queue_.emplace(std::move(write_task_pointer));
  DoWrite();
}

void Context::DoConnectHost() {
  LOG_INFO("Connecting to " << host_name_);
  auto& socket =
      server_socket_.emplace<tcp::socket>(Engine::get().GetExecutor());
  auto handler = [this, &for_stream = socket](
                     const boost::system::error_code& error,
                     const tcp::endpoint& /*endpoint*/) {
    if (!for_stream.lowest_layer().is_open()) {
      return;
    }
    if (error) {
      LOG_ERROR(<< host_name_ << " failed to connect: " << error.message());
      return;
    }
    if (status_ != Status::CLIENT_CONNECTED) {
      LOG_ERROR(<< host_name_ << " status mismatch " << status_);
      return;
    }
    auto handler = std::bind(&Context::HandleServerMessage, shared_from_this(),
                             std::placeholders::_1, std::placeholders::_2,
                             std::placeholders::_3);
    server_message_reader_.Start(for_stream, handler);
    DoWrite();
    status_ = Status::CLIENT_HELLO_FORWARDED;
  };

  boost::asio::async_connect(socket.lowest_layer(), host_endpoints_,
                             std::move(handler));
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
  } else {
    if (std::holds_alternative<tcp::socket>(server_socket_)) {
      auto& socket = std::get<tcp::socket>(server_socket_);
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