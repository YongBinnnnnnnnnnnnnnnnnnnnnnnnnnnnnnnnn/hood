#include <boost/asio.hpp>

#include "engine.hpp"
#include "network.hpp"

namespace hood_proxy {
namespace network {

static const auto local_network_0 =
    boost::asio::ip::make_network_v4("0.0.0.0/8");
static const auto local_network_10 =
    boost::asio::ip::make_network_v4("10.0.0.0/8");
static const auto local_network_10064 =
    boost::asio::ip::make_network_v4("100.64.0.0/10");
static const auto local_network_127 =
    boost::asio::ip::make_network_v4("127.0.0.0/8");
static const auto local_network_169254 =
    boost::asio::ip::make_network_v4("169.254.0.0/16");
static const auto local_network_17216 =
    boost::asio::ip::make_network_v4("172.16.0.0/12");
static const auto local_network_192 =
    boost::asio::ip::make_network_v4("192.0.0.0/24");
static const auto local_network_19202 =
    boost::asio::ip::make_network_v4("192.0.2.0/24");
static const auto local_network_1928899 =
    boost::asio::ip::make_network_v4("192.88.99.0/24");
static const auto local_network_192168 =
    boost::asio::ip::make_network_v4("192.168.0.0/16");
static const auto local_network_192188 =
    boost::asio::ip::make_network_v4("192.18.0.0/15");
static const auto local_network_19851100 =
    boost::asio::ip::make_network_v4("192.51.100.0/24");
static const auto local_network_2030113 =
    boost::asio::ip::make_network_v4("203.0.113.0/24");
static const auto local_network_233252 =
    boost::asio::ip::make_network_v4("233.252.0.0/24");
static const auto local_network_240 =
    boost::asio::ip::make_network_v4("240.0.0.0/4");
static const auto local_network_255255255255 =
    boost::asio::ip::make_network_v4("255.255.255.255/32");

static inline bool is_address_in_network(
    const boost::asio::ip::network_v4& network,
    const boost::asio::ip::address_v4& address) {
  return network.address().to_uint() ==
         (address.to_uint() & network.netmask().to_uint());
}

bool IsLocalAddress(const boost::asio::ip::address_v4& address) {
  return is_address_in_network(local_network_0, address) ||
         is_address_in_network(local_network_10, address) ||
         is_address_in_network(local_network_10064, address) ||
         is_address_in_network(local_network_127, address) ||
         is_address_in_network(local_network_169254, address) ||
         is_address_in_network(local_network_17216, address) ||
         is_address_in_network(local_network_192, address) ||
         is_address_in_network(local_network_19202, address) ||
         is_address_in_network(local_network_1928899, address) ||
         is_address_in_network(local_network_192168, address) ||
         is_address_in_network(local_network_192188, address) ||
         is_address_in_network(local_network_19851100, address) ||
         is_address_in_network(local_network_2030113, address) ||
         is_address_in_network(local_network_233252, address) ||
         is_address_in_network(local_network_240, address) ||
         is_address_in_network(local_network_255255255255, address);
}

}  // namespace network
}  // namespace hood_proxy