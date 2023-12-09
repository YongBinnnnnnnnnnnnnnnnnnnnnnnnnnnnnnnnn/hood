#ifndef HOOD_NAME_SERVICE_CLIENT_H_
#define HOOD_NAME_SERVICE_CLIENT_H_
#include <boost/asio.hpp>
#include <memory>
#include <string>
#include <vector>

namespace hood {
namespace name_service {

using ResultHandler =
    std::function<void(const boost::system::error_code& error,
                       const std::vector<boost::asio::ip::address>& results)>;
void Resolve(const std::string& host_name, ResultHandler&& handler);

}  // namespace name_service
}  // namespace hood
#endif  // HOOD_NAME_SERVICE_CLIENT_H_