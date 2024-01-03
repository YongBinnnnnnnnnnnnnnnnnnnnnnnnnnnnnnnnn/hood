#include <boost/asio.hpp>
#include <fstream>
#include <string>

#include "configuration.hpp"
#include "logging.hpp"
#include "network.hpp"

namespace hood_proxy {
boost::program_options::variables_map Configuration::variables_;

static bool LoadConfigurationLines(
    const char* file_name, std::function<void(const std::string&)> callback) {
  std::fstream stream{file_name, stream.in};
  if (!stream.is_open()) {
    LOG_ERROR("Failed to read " << file_name);
    return false;
  }
  for (std::string line; std::getline(stream, line);) {
    if (line.length() == 0 || line[0] == '#') {
      continue;
    }
    callback(line);
  }
  return true;
}

int Configuration::LoadHoodSystemConfiguration() {
  bool success = true;
  {
    std::vector<boost::asio::ip::network_v4> blocked_subnets;
    success = LoadConfigurationLines(
        "/var/lib/hood/ip_subnet_blacklist.txt",
        [&blocked_subnets](const std::string& line) {
          blocked_subnets.emplace_back(boost::asio::ip::make_network_v4(line));
        });
    variables_.emplace("hood_ip_subset_blacklist",
                       boost::program_options::variable_value{
                           std::move(blocked_subnets), false});
    if (!success) {
      return 1;
    }
  }
  return 0;
}

}  // namespace hood_proxy