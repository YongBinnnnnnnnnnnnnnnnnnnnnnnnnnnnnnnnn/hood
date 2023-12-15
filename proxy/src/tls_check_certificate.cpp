#include <boost/algorithm/string/predicate.hpp>
#include <boost/endian/conversion.hpp>
#include <chrono>
#include <map>
#include <set>

#include "configuration.hpp"
#include "engine.hpp"
#include "logging.hpp"
#include "name_service_client.hpp"
#include "network.hpp"
#include "tls_check_certificate.hpp"
#include "tls_check_certificate_worker.hpp"

namespace ssl = boost::asio::ssl;
namespace endian = boost::endian;
using boost::algorithm::ends_with;
using boost::asio::async_read;
using boost::asio::async_write;
using boost::asio::ip::make_address;
using boost::asio::ip::tcp;
using boost::system::error_code;
using std::chrono::milliseconds;
using std::chrono::seconds;
using steady_clock = std::chrono::steady_clock;

namespace hood_proxy {
namespace tls {

// TODO use hash as key to prevent privacy leak

static const seconds cache_lifespan_(300);
static const size_t cache_size_limit_(4096);

struct ResultCache {
  std::vector<tcp::endpoint> trusted_endpoints;
  steady_clock::time_point creation_time;
};

static thread_local std::map<std::string, ResultCache> cached_results_;

static thread_local std::map<std::string,
                             std::vector<CheckCertificateResultHandler>>
    pending_handlers_;

void WorkerResultHandler(const std::string& host_name,
                         const boost::asio::ip::tcp::endpoint& endpoint,
                         uintptr_t flags) {
  if (flags & CertificateCheckWorker::Flags::good) {
    const auto now = steady_clock::now();
    auto pair = cached_results_.find(host_name);
    if (pair == cached_results_.end() &&
        cached_results_.size() > cache_size_limit_) {
      auto released_amount =
          std::erase_if(cached_results_, [now](const auto& item) {
            auto age = now - item.second.creation_time;
            return age > cache_lifespan_;
          });
      if (released_amount == 0) {
        cached_results_.erase(cached_results_.begin());
      }
    }
    ResultCache* cache;
    if (pair == cached_results_.end()) {
      cache = &cached_results_[host_name];
    } else {
      cache = &pair->second;
    }
    cache->creation_time = now;

    cache->trusted_endpoints.push_back(endpoint);
  }

  if (flags & (CertificateCheckWorker::Flags::good |
               CertificateCheckWorker::Flags::finished)) {
    auto pair = pending_handlers_.find(host_name);
    if (pair == pending_handlers_.end()) {
      return;
    }
    std::vector<tcp::endpoint> empty_vector;
    auto endpoints = &empty_vector;
    {
      auto pair = cached_results_.find(host_name);
      if (pair != cached_results_.end()) {
        endpoints = &pair->second.trusted_endpoints;
      }
    }
    auto& handlers = pair->second;
    for (auto& handler : handlers) {
      handler(*endpoints);
    }
    pending_handlers_.erase(pair);
    return;
  }
}

static void DoResolve(std::string host_name) {
  LOG_INFO("Resolving " << host_name);
  hood::name_service::Resolve(
      host_name,
      [host_name](const boost::system::error_code& error,
                  const std::vector<boost::asio::ip::address>& results) {
        tcp::endpoint dummy_endpoint;
        constexpr const uintptr_t error_finish_flags =
            (CertificateCheckWorker::Flags::error |
             CertificateCheckWorker::Flags::finished);
        if (error) {
          LOG_INFO("Error while resolving " << host_name << " "
                                            << error.message());
          WorkerResultHandler(host_name, dummy_endpoint, error_finish_flags);
          return;
        }
        std::vector<tcp::endpoint> endpoints;
        for (const auto& result : results) {
          endpoints.emplace_back(tcp::endpoint(result, 443));
        }
        if (endpoints.size() == 0) {
          LOG_INFO("Unable to resolve " << host_name);
          WorkerResultHandler(host_name, dummy_endpoint, error_finish_flags);
          return;
        }
        network::RemoveV6AndLocalEndpoints(endpoints);
        if (endpoints.size() == 0) {
          if (!ends_with(host_name, Configuration::proxy_domain)) {
            DoResolve(host_name + Configuration::proxy_domain.data());
            return;
          }
          LOG_INFO(<< host_name << " endpoints are not acceptable");
          WorkerResultHandler(host_name, dummy_endpoint, error_finish_flags);
          return;
        }

#ifdef NO_CHECK_CERTIFICATE
        WorkerResultHandler(host_name, endpoints[0], CertificateCheckWorker::Flags::good |
             CertificateCheckWorker::Flags::finished);
             return;
#endif
        auto worker = CertificateCheckWorker::create(host_name, endpoints,
                                                     WorkerResultHandler);
        worker->Start();
      });
}

void CheckCertificateOf(const std::string& host_name,
                        CheckCertificateResultHandler&& handler) {
  {
    auto pair = cached_results_.find(host_name);
    if (pair != cached_results_.end()) {
      auto age = steady_clock::now() - pair->second.creation_time;
      if (age < cache_lifespan_) {
        handler(pair->second.trusted_endpoints);
        return;
      } else {
        cached_results_.erase(pair);
      }
    }
  }

  {
    auto pair = pending_handlers_.find(host_name);
    if (pair != pending_handlers_.end() && pair->second.size()) {
      pair->second.emplace_back(std::move(handler));
      return;
    }
    pending_handlers_[host_name].emplace_back(std::move(handler));
  }

  DoResolve(host_name);
}

}  // namespace tls
}  // namespace hood_proxy
