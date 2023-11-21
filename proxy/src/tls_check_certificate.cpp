#include <boost/algorithm/string/predicate.hpp>
#include <boost/endian/conversion.hpp>
#include <chrono>
#include <unordered_map>

#include "configuration.hpp"
#include "engine.hpp"
#include "logging.hpp"
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

static const seconds cache_lifespan_(300);
static thread_local boost::asio::ip::tcp::resolver resolver_(
    Engine::get().GetExecutor());

struct ResultCache {
  std::vector<tcp::endpoint> trusted_endpoints;
  steady_clock::time_point creation_time;
};

static thread_local std::unordered_map<std::string, ResultCache>
    cached_results_;

static thread_local std::unordered_map<
    std::string, std::vector<CheckCertificateResultHandler>>
    pending_handlers_;

void WorkerResultHandler(const std::string& host_name,
                         boost::asio::ip::tcp::endpoint& endpoint,
                         uintptr_t flags) {
  if (flags & CertificateCheckWorker::Flags::good) {
    auto& cache = cached_results_[host_name];
    cache.creation_time = steady_clock::now();
    cache.trusted_endpoints.emplace_back(endpoint);
    if (cache.trusted_endpoints.size() >= 4) {
      auto pair = pending_handlers_.find(host_name);
      if (pair == pending_handlers_.end()) {
        return;
      }

      auto& handlers = pair->second;
      for (auto& handler : handlers) {
        handler(cache.trusted_endpoints);
      }
      pending_handlers_.erase(pair);
      return;
    }
  }

  if (flags & CertificateCheckWorker::Flags::finished) {
    auto pair = pending_handlers_.find(host_name);
    if (pair == pending_handlers_.end()) {
      return;
    }

    auto& handlers = pair->second;
    std::vector<tcp::endpoint> empty_vector;
    for (auto& handler : handlers) {
      handler(empty_vector);
    }
    pending_handlers_.erase(pair);
    return;
  }
}

void DoResolve(const std::string& host_name) {
  LOG_INFO("Resolving " << host_name);
  tcp::resolver::query name_query(host_name, "https");

  resolver_.async_resolve(
      name_query, [host_name](const boost::system::error_code& error,
                              tcp::resolver::results_type results) {
        if (error) {
          LOG_INFO("Error while resolving " << host_name << " "
                                            << error.message());
          return;
        }
        std::vector<tcp::endpoint> endpoints;
        typeof(results) end;
        for (; results != end; results++) {
          endpoints.emplace_back(results->endpoint());
        }
        if (endpoints.size() == 0) {
          LOG_INFO("Unable to resolve " << host_name);
          // TODO
          return;
        }
        network::RemoveV6AndLocalEndpoints(endpoints);
        if (endpoints.size() == 0) {
          if (!ends_with(host_name, Configuration::proxy_domain)) {
            DoResolve(host_name + Configuration::proxy_domain.data());
            return;
          }
          LOG_INFO(<< host_name << " endpoints are not acceptable");
          // TODO
          return;
        }

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
      } else {
        pair->second.trusted_endpoints.clear();
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