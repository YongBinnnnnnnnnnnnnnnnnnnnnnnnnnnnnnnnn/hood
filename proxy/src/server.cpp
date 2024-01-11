#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <chrono>
#include <csignal>
#include <iostream>
#include <string>

#include "configuration.hpp"
#include "engine.hpp"
#include "logging.hpp"
#include "server.hpp"
#include "tls_context.hpp"

using boost::asio::any_completion_executor;
using boost::asio::signal_set;
using boost::asio::ip::make_address;
using boost::asio::ip::tcp;
using boost::asio::ip::udp;
using boost::system::error_code;
using std::cout;
using std::endl;
using std::function;
using std::string;
using std::chrono::milliseconds;

namespace hood_proxy {

Server::Server()
    : io_context_(Engine::get().GetExecutor()),
      acceptor_(io_context_.get_executor()),
      listen_address_(
          make_address(Configuration::get("listen-address").as<string>())),
      listen_port_(Configuration::get("listen-port").as<uint16_t>()),
      tls_proxy_acceptor_(io_context_),
      tls_proxy_address_(
          make_address(Configuration::get("tls-proxy-address").as<string>())),
      stop_(false) {}

void Server::Run() { io_context_.run(); }

void Server::StartUdp() {
  auto udp_endpoint = udp::endpoint(listen_address_, listen_port_);
  udp::socket socket(io_context_);
  socket.open(udp_endpoint.protocol());
  socket.bind(udp_endpoint);
  LOG_INFO("Listening on " << listen_address_ << ":" << listen_port_ << " UDP");
}

void Server::StartTcp() {
  auto tcp_endpoint = tcp::endpoint(listen_address_, listen_port_);
  acceptor_.open(tcp_endpoint.protocol());
  acceptor_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
  acceptor_.bind(tcp_endpoint);
  acceptor_.listen();
  DoAccept();
  LOG_INFO("Listening on " << listen_address_ << ":" << listen_port_ << " TCP");
}

void Server::StartTls() {
  auto& ports =
      Configuration::get("tls-proxy-port").as<std::vector<uint16_t>>();
  for (auto port : ports) {
    auto tls_endpoint = tcp::endpoint(tls_proxy_address_, port);
    tls_proxy_acceptor_.open(tls_endpoint.protocol());
    tls_proxy_acceptor_.set_option(
        boost::asio::ip::tcp::acceptor::reuse_address(true));
    tls_proxy_acceptor_.bind(tls_endpoint);
    tls_proxy_acceptor_.listen();
    DoAcceptTls();
    LOG_INFO("Listening on " << tls_proxy_address_ << ":" << port
                             << " TLS TCP");
  }
}

void Server::DoAcceptTls() {
  tls_proxy_acceptor_.async_accept(
      [this](error_code error, tcp::socket socket) {
        if (stop_) {
          return;
        }
        if (!error) {
          //
          auto context = tls::Context::create(tls::Context::Role::client);
          context->Start(std::move(socket));
        }

        DoAcceptTls();
      });
}

void Server::DoAccept() {
  acceptor_.async_accept([this](error_code error, tcp::socket /*socket*/) {
    if (stop_) {
      return;
    }
    if (!error) {
      //
    }

    DoAccept();
  });
}

}  // namespace hood_proxy
