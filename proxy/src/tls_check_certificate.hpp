#ifndef HOOD_TLS_CHECK_CERTIFICATE_H_
#define HOOD_TLS_CHECK_CERTIFICATE_H_
#include <boost/asio.hpp>
#include <memory>
#include <string>
#include <vector>

namespace hood_proxy {
namespace tls {
using CheckCertificateResultHandler = std::function<void(
    const std::vector<boost::asio::ip::tcp::endpoint>& trusted_endpoints)>;
void CheckCertificateOf(const std::string& host_name,
                        CheckCertificateResultHandler&& handler);
}  // namespace tls
}  // namespace hood_proxy
#endif  // HOOD_TLS_CHECK_CERTIFICATE_H_