#ifndef HOOD_PROXY_SERVER_H_
#define HOOD_PROXY_SERVER_H_
#include <boost/asio.hpp>
#include <iostream>
#include <vector>

namespace hood_proxy {

class Server {
 public:
  Server();
  void StartUdp();
  void StartTcp();
  void Run();

 private:
  boost::asio::io_context& io_context_;
  boost::asio::ip::tcp::socket tcp_socket_;
  boost::asio::ip::tcp::acceptor acceptor_;
  boost::asio::signal_set signals_;
  boost::asio::ip::address listen_address_;
  uint16_t listen_port_;

  bool stop_;

  void DoAccept();
  void DoAwaitStop();
};

}  // namespace hood_proxy
#endif  // HOOD_PROXY_SERVER_H_