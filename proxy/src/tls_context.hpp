#ifndef HOOD_PROXY_TLS_CONTEXT_H_
#define HOOD_PROXY_TLS_CONTEXT_H_

#include <boost/asio.hpp>
#include <cassert>
#include <chrono>
#include <functional>
#include <memory>
#include <queue>
#include <vector>
#include <atomic>

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
  using SocketSharedPtr = std::shared_ptr<boost::asio::ip::tcp::socket>;

  enum class Role { client, server };
  static pointer create(Role role) { return pointer(new Context(role)); }
  
  struct WriteTask {
    Message message;
    std::vector<uint8_t> raw_message;
  };
  
  template <typename TransportType>
  void Start(TransportType&& socket) {
    auto& executor = Engine::get().GetExecutor();
    while (active_instance_counter_ > 10000) {
      executor.run_one();
    }
    active_instance_counter_++;
    auto socket_ptr = std::make_shared<boost::asio::ip::tcp::socket>(std::move(socket));
    socket_ = socket_ptr;
    static_assert(!std::is_same<TransportType, nullptr_t>::value,
                  "Can not start a proxy context with nullptr");
    TlsMessageReader::StreamHandlerTypeExample handler;
    if (role_ == Role::client) {
      handler = std::bind(&Context::HandleUserMessage, shared_from_this(),
                             std::placeholders::_1, std::placeholders::_2,
                             std::placeholders::_3);
    }
    else {
      handler = std::bind(&Context::HandleServerMessage, shared_from_this(),
                             std::placeholders::_1, std::placeholders::_2,
                             std::placeholders::_3);
    }
    message_reader_.Start(socket_ptr, std::move(handler));
    DoWrite();
  }
  void Pair(pointer another);
  void Queue(pointer from, std::shared_ptr<WriteTask> task);
  
  void Stop();

  ~Context();

 private:
  static std::atomic<intptr_t> active_instance_counter_;
  std::variant<nullptr_t, SocketSharedPtr> socket_;
  TlsMessageReader message_reader_;
  std::string host_name_;
  Role role_;
  pointer paired_;

  struct Flags {
    static constexpr uintptr_t CONNECTED_TO_HOST = 0b0001;
    static constexpr uintptr_t CLIENT_ENABLED_ENCRYPTION = 0b0010;
    static constexpr uintptr_t SERVER_ENABLED_ENCRYPTION = 0b0100;
  };
  uintptr_t flags_ = 0;

  std::queue<std::shared_ptr<WriteTask>> write_task_queue_;

  bool writing_ = false;

  Context(Role role)
      : socket_(nullptr),
        message_reader_(),
        role_(role),
        paired_(nullptr) {}
  TlsMessageReader::NextStep HandleUserMessage(TlsMessageReader::Reason reason,
                                               const uint8_t* data,
                                               uint16_t data_size);
  void DoConnectHost(const std::vector<boost::asio::ip::tcp::endpoint>& endpoints);
  TlsMessageReader::NextStep HandleServerMessage(
      TlsMessageReader::Reason reason, const uint8_t* data, uint16_t data_size);
  void LimitTaskQueueSize();
  void DoWrite();
};

}  // namespace tls
}  // namespace hood_proxy
#endif  // HOOD_PROXY_TLS_CONTEXT_H_
