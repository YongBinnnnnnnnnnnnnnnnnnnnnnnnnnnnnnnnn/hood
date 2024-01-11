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
  void StartTls();
  void Run();

 private:
  boost::asio::io_context& io_context_;
  boost::asio::executor executor_;
  boost::asio::ip::tcp::acceptor acceptor_;
  boost::asio::ip::address listen_address_;
  uint16_t listen_port_;
  std::vector<boost::asio::ip::tcp::acceptor> tls_proxy_acceptors_;
  boost::asio::ip::address tls_proxy_address_;

  bool stop_;

  void DoAccept();
  void DoAcceptTls(boost::asio::ip::tcp::acceptor& acceptor);
  void DoAwaitStop();
};

}  // namespace hood_proxy
#endif  // HOOD_PROXY_SERVER_H_
