#include <unistd.h>
#include <iostream>
#include "configuration.hpp"
#include "logging.hpp"
#include "server.hpp"
#include "version.h"

using boost::asio::ip::udp;
using hood_proxy::Configuration;
using hood_proxy::InitLogging;
using hood_proxy::Server;
using std::cout;

int main(int argc, const char **argv) {
  InitLogging();
  LOG_INFO("hood-proxy version:" << HOOD_PROXY_VERSION << " pid:" << getpid());
  auto result = Configuration::init(argc, argv);
  if (result < 0) {
    return result;
  }

  Server server;
  server.Run();
  LOG_INFO("Exit.");
}