#ifndef HOOD_PROXY_TLS_CONTEXT_H_
#define HOOD_PROXY_TLS_CONTEXT_H_

#include <boost/asio.hpp>
#include <chrono>
#include <functional>
#include <memory>
#include <queue>
#include <vector>

#include "configuration.hpp"
#include "message_reader.hpp"
#include "tls_defination_raw.hpp"

namespace hood_proxy {
namespace tls {

class Context : public std::enable_shared_from_this<Context> {
 public:
  using pointer = std::shared_ptr<Context>;
  using nullptr_t = std::nullptr_t;
  using TlsMessageReader = MessageReader<protocol::TLSPlaintext>;

  static pointer create() { return pointer(new Context()); }

  template <typename TransportType>
  void Start(TransportType&& socket) {
    using TcpSocketType = boost::asio::ip::tcp::socket;
    client_socket_ = std::move(socket);
    static_assert(!std::is_same<TransportType, nullptr_t>::value,
                  "Can not start a proxy context with nullptr");
    auto handler = std::bind(&Context::HandleUserMessage, shared_from_this(),
                             std::placeholders::_1, std::placeholders::_2,
                             std::placeholders::_3);
    client_message_reader_.Start(std::get<TcpSocketType>(client_socket_),
                                 handler);
  }
  void Stop();

  ~Context();

 private:
  std::variant<nullptr_t, boost::asio::ip::tcp::socket> client_socket_;
  TlsMessageReader client_message_reader_;
  std::variant<nullptr_t, boost::asio::ip::tcp::socket> server_socket_;
  TlsMessageReader server_message_reader_;
  enum class Status {
    CLIENT_CONNECTED,
    CLIENT_HELLO_FORWARDED,
    SERVER_HELLO_FORWARDED,
    WRITING,
  } status_ = Status::CLIENT_CONNECTED;

  bool writing_ = false;

  Context() {}
  void HandleUserMessage(TlsMessageReader::Reason reason, const uint8_t* data,
                         uint16_t data_size);
  void HandleServerMessage(TlsMessageReader::Reason reason, const uint8_t* data,
                           uint16_t data_size);
  void DoWrite();
};

}  // namespace tls
}  // namespace hood_proxy
#endif  // HOOD_PROXY_TLS_CONTEXT_H_