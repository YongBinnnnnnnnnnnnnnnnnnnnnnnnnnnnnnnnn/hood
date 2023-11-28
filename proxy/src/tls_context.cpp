#include <boost/algorithm/string/predicate.hpp>
#include <boost/endian/conversion.hpp>
#include <chrono>
#include <iostream>
#include <vector>

#include "logging.hpp"
#include "network.hpp"
#include "tls_check_certificate.hpp"
#include "tls_context.hpp"
#include "tls_message_decoder.hpp"
#include "tls_message_encoder.hpp"
#include "tls_security_policy.hpp"

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

Context::TlsMessageReader::NextStep Context::HandleUserMessage(
    TlsMessageReader::Reason reason, const uint8_t* data, uint16_t data_size) {
  auto next_step = TlsMessageReader::NextStep::Read;
  if (reason != TlsMessageReader::Reason::NEW_MESSAGE) {
    LOG_TRACE("ignore reason:" << static_cast<int>(reason));
    return next_step;
  }
  if (!data || !data_size) {
    LOG_ERROR("empty message!");
    return next_step;
  }
  LOG_TRACE("received query");
  size_t decoded_message_size = 0;
  auto write_task_pointer = std::make_unique<WriteTask>();
  write_task_pointer->to_client = false;
  auto& message = write_task_pointer->message;
  write_task_pointer->raw_message.insert(write_task_pointer->raw_message.end(),
                                         data, data + data_size);

  if (flags_ & Flags::CLIENT_ENABLED_ENCRYPTION) {
    write_task_queue_.emplace(std::move(write_task_pointer));
    DoWrite();
    return next_step;
  }

  auto decode_result = MessageDecoder::DecodeMesssage(message, data, data_size,
                                                      decoded_message_size);
  if (decode_result != MessageDecoder::ResultType::good) {
    LOG_ERROR("decode failed!");
    Stop();
    return next_step;
  }

  if (message.type == protocol::ContentType::handshake) {
    LOG_INFO("Received handshake");
    if (message.legacy_record_version < protocol::Version::TLS_1_0) {
      LOG_INFO("Discard connection due to protocol version"
               << message.legacy_record_version);
      Stop();
      return next_step;
    }
    auto& handshake_messages = std::get<handshake::Messages>(message.content);
    for (auto& handshake_message : handshake_messages) {
      if (handshake_message.type == protocol::handshake::Type::client_hello) {
        auto& client_hello_message =
            std::get<handshake::ClientHello>(handshake_message.content);

        if (!SecurityPolicy::HardenVersions(handshake_message)) {
          LOG_INFO("Discard connection due to protocol version"
                   << message.legacy_record_version);
          Stop();
          return next_step;
        }
        if (!SecurityPolicy::CheckCipherSuites(
                client_hello_message.cipher_suites)) {
          LOG_INFO("Discard connection due to unsafe cipher suites "
                   << host_name_);
          // TODO
          Stop();
          return next_step;
        }
        std::string host_name;
        extension::FindHostName(host_name, client_hello_message.extensions);

        write_task_queue_.emplace(std::move(write_task_pointer));
        if (flags_ & Flags::CONNECTED_TO_HOST) {
          if (host_name != host_name_) {
            LOG_INFO("Discard connection due to host name mismatch "
                     << host_name << " " << host_name_);
            Stop();
            return next_step;
          }
          DoWrite();
        } else {
          if (host_name.length() == 0) {
            LOG_INFO("Discard connection due to lack of host name");
            Stop();
            return next_step;
          }
          host_name_ = host_name;
          CheckCertificateOf(
              host_name_, [this, _ = shared_from_this()](
                              const std::vector<boost::asio::ip::tcp::endpoint>&
                                  trusted_endpoints) {
                if (trusted_endpoints.empty()) {
                  LOG_ERROR("Secure connection failed " << host_name_);
                  return;
                }
                host_endpoints_ = trusted_endpoints;

                DoConnectHost();
              });
        }

        return next_step;
      }
    }
  } else if (message.type == protocol::ContentType::change_cipher_spec) {
    flags_ |= Flags::CLIENT_ENABLED_ENCRYPTION;
  }
  write_task_queue_.emplace(std::move(write_task_pointer));
  DoWrite();
  return next_step;
}

Context::TlsMessageReader::NextStep Context::HandleServerMessage(
    TlsMessageReader::Reason reason, const uint8_t* data, uint16_t data_size) {
  auto next_step = TlsMessageReader::NextStep::Read;
  if (reason != TlsMessageReader::Reason::NEW_MESSAGE) {
    LOG_TRACE("ignore reason:" << static_cast<int>(reason));
    Stop();
    return next_step;
  }
  if (!data || !data_size) {
    LOG_ERROR("empty message!");
    Stop();
    return next_step;
  }
  LOG_TRACE("received query");

  size_t decoded_message_size = 0;
  auto write_task_pointer = std::make_unique<WriteTask>();
  write_task_pointer->to_client = true;
  auto& message = write_task_pointer->message;
  write_task_pointer->raw_message.insert(write_task_pointer->raw_message.end(),
                                         data, data + data_size);

  if (flags_ & Flags::SERVER_ENABLED_ENCRYPTION) {
    write_task_queue_.emplace(std::move(write_task_pointer));
    DoWrite();
    return next_step;
  }

  auto decode_result = MessageDecoder::DecodeMesssage(message, data, data_size,
                                                      decoded_message_size);
  if (decode_result != MessageDecoder::ResultType::good) {
    LOG_ERROR("decode failed!");
    Stop();
    return next_step;
  }

  if (message.type == protocol::ContentType::handshake) {
    LOG_INFO("Received handshake");
    if (message.legacy_record_version < protocol::Version::TLS_1_0) {
      LOG_INFO("Discard connection due to protocol version"
               << message.legacy_record_version);
      Stop();
      return next_step;
    }
    auto& handshake_messages = std::get<handshake::Messages>(message.content);
    for (auto& handshake_message : handshake_messages) {
      if (handshake_message.type == protocol::handshake::Type::server_hello) {
        auto& server_hello_message =
            std::get<handshake::ServerHello>(handshake_message.content);
        if (server_hello_message.hello_retry_type ==
                handshake::HelloRetryType::FOR_TLS_1_1 ||
            server_hello_message.hello_retry_type ==
                handshake::HelloRetryType::FOR_UNKNOWN) {
          LOG_INFO("Discard connection due to protocol version"
                   << message.legacy_record_version);
          Stop();
          return next_step;
        } else if (server_hello_message.hello_retry_type ==
                   handshake::HelloRetryType::NOT_RETRY) {
        }

        if (!SecurityPolicy::allowed_cipher_suites.contains(
                server_hello_message.cipher_suite)) {
          LOG_INFO("Discard connection due to unsafe cipher suite"
                   << server_hello_message.cipher_suite);
        }
        break;
      }
    }

    write_task_queue_.emplace(std::move(write_task_pointer));
    DoWrite();
    return next_step;
  } else if (message.type == protocol::ContentType::change_cipher_spec) {
    flags_ |= Flags::SERVER_ENABLED_ENCRYPTION;
  }

  write_task_queue_.emplace(std::move(write_task_pointer));
  DoWrite();
  return next_step;
}

void Context::DoConnectHost() {
  LOG_INFO("Connecting to " << host_name_);
  auto& socket =
      server_socket_.emplace<tcp::socket>(Engine::get().GetExecutor());
  auto handler = [this, _ = shared_from_this(), &for_stream = socket](
                     const boost::system::error_code& error,
                     const tcp::endpoint& /*endpoint*/) {
    if (!for_stream.lowest_layer().is_open()) {
      Stop();
      return;
    }
    if (error) {
      LOG_ERROR(<< host_name_ << " failed to connect: " << error.message());
      Stop();
      return;
    }

    auto handler = std::bind(&Context::HandleServerMessage, shared_from_this(),
                             std::placeholders::_1, std::placeholders::_2,
                             std::placeholders::_3);
    server_message_reader_.Start(for_stream, handler);
    DoWrite();
    flags_ |= Flags::CONNECTED_TO_HOST;
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