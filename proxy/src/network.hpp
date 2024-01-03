#ifndef HOOD_PROXY_NETWORK_H_
#define HOOD_PROXY_NETWORK_H_

#include <boost/asio.hpp>
#include <chrono>
#include <functional>
#include <memory>
#include <queue>
#include <vector>

#include "configuration.hpp"
#include "engine.hpp"

namespace hood_proxy {
namespace network {

bool IsLocalAddress(const boost::asio::ip::address_v4& address);
bool IsBlockedAddress(const boost::asio::ip::address_v4& address);

static inline void RemoveV6AndBlockedEndpoints(
    std::vector<boost::asio::ip::tcp::endpoint>& endpoints) {
  endpoints.erase(
      std::remove_if(endpoints.begin(), endpoints.end(),
                     [](const boost::asio::ip::tcp::endpoint& endpoint) {
                       auto address = endpoint.address();
                       return address.is_v6() ||
                              IsBlockedAddress(address.to_v4());
                     }),
      endpoints.end());
}
}  // namespace network
}  // namespace hood_proxy
#endif  // HOOD_PROXY_NETWORK_H_