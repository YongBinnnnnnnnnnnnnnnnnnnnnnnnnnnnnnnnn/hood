#ifndef HOOD_TLS_CHECK_CERTIFICATE_WORKER_H_
#define HOOD_TLS_CHECK_CERTIFICATE_WORKER_H_
#include <atomic>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <memory>
#include <string>
#include <unordered_map>

namespace hood_proxy {
namespace tls {
class CertificateCheckWorker
    : public std::enable_shared_from_this<CertificateCheckWorker> {
 public:
  struct Flags {
    static constexpr uintptr_t good = 0b0001;
    static constexpr uintptr_t error = 0b0010;
    static constexpr uintptr_t finished = 0b0100;
  };

  using pointer = std::shared_ptr<CertificateCheckWorker>;

  using tcp_endpoints_type = std::vector<boost::asio::ip::tcp::endpoint>;
  using ResultHandler = std::function<void(
      const std::string& host_name,
      const boost::asio::ip::tcp::endpoint& endpoint, uintptr_t flags)>;
  static pointer create(const std::string& host_name,
                        const tcp_endpoints_type& endpoints,
                        ResultHandler handler) {
    return pointer(
        new CertificateCheckWorker(host_name, endpoints, std::move(handler)));
  }
  void Start();

 private:
  CertificateCheckWorker(const std::string& host_name,
                         const tcp_endpoints_type& endpoints,
                         ResultHandler handler);

  void CheckEndpoint(boost::asio::ip::tcp::endpoint& endpoint);
  void CallHandler(const boost::asio::ip::tcp::endpoint& endpoint,
                   uintptr_t flags);
  std::string host_name_;
  tcp_endpoints_type endpoints_;
  std::atomic<size_t> callback_countdown_;
  ResultHandler handler_;
};
}  // namespace tls
}  // namespace hood_proxy
#endif  // HOOD_TLS_CHECK_CERTIFICATE_WORKER_H_