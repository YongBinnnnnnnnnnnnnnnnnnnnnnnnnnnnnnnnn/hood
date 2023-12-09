#ifndef HOOD_PROXY_CONFIGURATION_H_
#define HOOD_PROXY_CONFIGURATION_H_

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>
#include <fstream>
#include <iostream>
#include <string>

namespace hood_proxy {

class Configuration {
 public:
  static constexpr const std::string_view proxy_domain =
      std::string_view(".hood-proxy");
  static constexpr const uintptr_t max_length_of_a_domain = 253;
  static inline const boost::program_options::variable_value& get(
      const std::string& name) {
    return variables_[name];
  }

  template <class charT>
  static int init(int argc, const charT* const argv[]) {
    namespace bpo = boost::program_options;
    using std::cerr;
    using std::cout;
    using std::string;

    bpo::options_description commandline_options("Commandline options");
    auto add_commandline_option = commandline_options.add_options();
    add_commandline_option("help", "produce help message");
    add_commandline_option(
        "config", bpo::value<string>()->default_value("./hood_proxy.conf"),
        "specify config file");
    bpo::store(bpo::parse_command_line(argc, argv, commandline_options),
               variables_);
    bpo::notify(variables_);

    if (variables_.count("help")) {
      cout << commandline_options << "\n";
      return 1;
    }

    bpo::options_description configurations("Configurations");
    auto add_configuration_option = configurations.add_options();
    add_configuration_option("tls-proxy-port",
                             bpo::value<uint16_t>()->default_value(443),
                             "tls proxy port");
    add_configuration_option("listen-port",
                             bpo::value<uint16_t>()->default_value(443),
                             "dns proxy port");
    add_configuration_option("tls-proxy-address",
                             bpo::value<string>()->default_value("127.0.0.1"),
                             "tls proxy address");
    add_configuration_option("listen-address",
                             bpo::value<string>()->default_value("0.0.0.0"),
                             "server listen address");
    add_configuration_option("query-timeout",
                             bpo::value<uint32_t>()->default_value(10000),
                             "timeout for every query in milliseconds");
    add_configuration_option(
        "remote-servers",
        bpo::value<string>()->default_value(
            "tls@853/1.0.0.1/cloudflare-dns.com"),
        "foreign dns server format:\n"
        "transport_type1[@port][|transport_type2][/address1][|address2]"
        "[/hostname],///,...");
    bpo::store(bpo::parse_config_file(variables_["config"].as<string>().c_str(),
                                      configurations),
               variables_);
    bpo::notify(variables_);
    return 0;
  }

 private:
  int ParseConfigurationFile();
  static boost::program_options::variables_map variables_;
};

}  // namespace hood_proxy

#endif  // HOOD_PROXY_CONFIGURATION_H_