#ifndef HOOD_PROXY_TLS_CONTEXT_H_
#define HOOD_PROXY_TLS_CONTEXT_H_

#include <boost/asio.hpp>
#include <cassert>
#include <chrono>
#include <functional>
#include <memory>
#include <queue>
#include <vector>

#include "configuration.hpp"
#include "engine.hpp"
#include "message_reader.hpp"
#include "tls_definition.hpp"

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
  boost::asio::ip::tcp::resolver resolver_;
  std::string host_name_;
  std::vector<boost::asio::ip::tcp::endpoint> host_endpoints_;
  std::variant<nullptr_t, boost::asio::ip::tcp::socket> server_socket_;
  TlsMessageReader server_message_reader_;
  struct Status {
    static constexpr uintptr_t CLIENT_CONNECTED = 0;
    static constexpr uintptr_t CLIENT_HELLO_FORWARDED = 1;
    static constexpr uintptr_t SERVER_HELLO_FORWARDED = 2;
  };
  uintptr_t status_ = Status::CLIENT_CONNECTED;

  struct Flags {
    static constexpr uintptr_t CLIENT_ENABLED_ENCRYPTION = 0b0001;
    static constexpr uintptr_t SERVER_ENABLED_ENCRYPTION = 0b0010;
  };
  uintptr_t flags_ = 0;

  struct WriteTask {
    bool to_client;
    Message message;
    std::vector<uint8_t> raw_message;
  };
  std::queue<std::unique_ptr<WriteTask>> write_task_queue_;

  bool writing_ = false;

  Context()
      : client_socket_(nullptr),
        client_message_reader_(),
        resolver_(Engine::get().GetExecutor()) {}
  void HandleUserMessage(TlsMessageReader::Reason reason, const uint8_t* data,
                         uint16_t data_size);
  void HandleClientHello(TlsMessageReader::Reason reason, const uint8_t* data,
                         uint16_t data_size);
  void DoResolveConnect(const std::string& host_name);
  void DoConnectHost();
  void HandleServerMessage(TlsMessageReader::Reason reason, const uint8_t* data,
                           uint16_t data_size);
  void DoWrite();
};

}  // namespace tls
}  // namespace hood_proxy
#endif  // HOOD_PROXY_TLS_CONTEXT_H_