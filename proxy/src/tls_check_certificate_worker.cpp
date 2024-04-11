#include <boost/predef/os.h>
#include <openssl/err.h>

#include <boost/endian/conversion.hpp>
#include <chrono>

#include "configuration.hpp"
#include "engine.hpp"
#include "logging.hpp"
#include "tls_check_certificate_worker.hpp"

namespace ssl = boost::asio::ssl;
namespace endian = boost::endian;
using boost::asio::async_read;
using boost::asio::async_write;
using boost::asio::ip::make_address;
using boost::asio::ip::tcp;
using boost::system::error_code;
using std::chrono::milliseconds;
using std::chrono::seconds;

namespace hood_proxy {
namespace tls {

// thread-unsafe, designed for thread_local use

static inline boost::asio::ssl::context BuildSSLContext() {
  // TODO: support more tls option from configuration
  // Use system cert
  boost::asio::ssl::context result(ssl::context::tls_client);
  boost::system::error_code error;
  result.set_default_verify_paths(error);
  // lowest tls version set to 1.2
  result.set_options(ssl::context::default_workarounds |
                     ssl::context::no_tlsv1 | ssl::context::no_tlsv1_1);
  SSL_CTX_set_session_cache_mode(result.native_handle(),
                                 SSL_SESS_CACHE_NO_INTERNAL);

  SSL_CTX_set_ciphersuites(result.native_handle(),
                           "ECDHE-RSA-AES128-GCM-SHA256"
                           ":ECDHE-RSA-AES256-GCM-SHA384"
                           ":ECDHE-ECDSA-AES128-GCM-SHA256"
                           ":ECDHE-ECDSA-AES256-GCM-SHA384"
                           ":TLS_AES_256_GCM_SHA384"
                           ":TLS_AES_128_GCM_SHA256"
                           ":TLS_CHACHA20_POLY1305_SHA256");
  return result;
}

static auto ssl_context_ = BuildSSLContext();

struct CertificateVerificationContext {
  using stream_type = boost::asio::ssl::stream<boost::asio::ip::tcp::socket>;
  stream_type socket;
  void Close() {
    if (!socket.lowest_layer().is_open()) {
      return;
    }

    boost::system::error_code error;
    socket.lowest_layer().shutdown(tcp::socket::shutdown_both, error);
    socket.lowest_layer().cancel(error);
    socket.lowest_layer().close();
  }
  CertificateVerificationContext()
      : socket(Engine::get().GetExecutor(), ssl_context_) {}
  ~CertificateVerificationContext() { Close(); }
};

CertificateCheckWorker::CertificateCheckWorker(
    const std::string& host_name, const tcp_endpoints_type& endpoints,
    ResultHandler handler)
    : host_name_(host_name),
      endpoints_(endpoints),
      callback_countdown_(endpoints.size()),
      handler_(std::move(handler)) {}

void CertificateCheckWorker::CheckEndpoint(
    boost::asio::ip::tcp::endpoint& endpoint) {
  LOG_INFO(<< host_name_);
  auto context = std::make_shared<CertificateVerificationContext>();

  auto& socket = context->socket;

  socket.set_verify_mode(ssl::verify_peer);

  {
    // Add Server Name Indication SNI
    SSL_set_tlsext_host_name(socket.native_handle(), host_name_.data());
  }

  socket.set_verify_callback(
      [host_name = host_name_, endpoint = endpoint](
          bool preverified, boost::asio::ssl::verify_context& ctx) {
        X509* cert = X509_STORE_CTX_get_current_cert(ctx.native_handle());
        if (cert) {
          char subject_name[512];
          X509_NAME_oneline(X509_get_subject_name(cert), subject_name,
                            sizeof(subject_name) - 1);
          LOG_INFO(" verifying " << host_name << " " << endpoint << " :"
                                 << subject_name);
        } else {
          LOG_INFO(" verifying " << host_name << " " << endpoint << " :"
                                 << "has no certificate");
        };
        return ssl::host_name_verification(host_name)(preverified, ctx);
      });

  auto connect_handler = [this, _ = shared_from_this(), context, &endpoint](
                             const boost::system::error_code& error,
                             const tcp::endpoint& /*endpoint*/) {
    if (!context->socket.lowest_layer().is_open()) {
      CallHandler(endpoint, Flags::error);
      return;
    }
    if (error) {
      LOG_ERROR(<< host_name_ << " connect failed: " << error.message());
      CallHandler(endpoint, Flags::error);
      return;
    }

    context->socket.async_handshake(
        boost::asio::ssl::stream_base::client,
        [this, _ = shared_from_this(), context,
         &endpoint](const boost::system::error_code& error) {
          if (!context->socket.lowest_layer().is_open()) {
            CallHandler(endpoint, Flags::error);
            return;
          }
          if (error) {
            if (error.category() == boost::asio::error::get_ssl_category()) {
              auto openssl_error = ERR_get_error();
              if (openssl_error) {
                char detail[256];
                ERR_error_string_n(openssl_error, detail, sizeof(detail));
                const char *file, *function, *data;
                int line, flags;
                if (ERR_get_error_all(&file, &line, &function, &data, &flags) ==
                    0 && line) {
                  if (flags & ERR_TXT_STRING) {
                    LOG_ERROR(<< host_name_ << " handshake failed: "
                              << "openssl " << detail << ":" << *file << ":"
                              << ":" << line << ":" << *function << ":" << *data);
                  } else {
                    LOG_ERROR(<< host_name_ << " handshake failed: "
                              << "openssl " << detail << ":" << *file << ":"
                              << ":" << line << ":" << *function);
                  }
                } else {
                  LOG_ERROR(<< host_name_ << " handshake failed: "
                            << "openssl " << detail);
                }
              }

            } else {
              LOG_ERROR(<< host_name_
                        << " handshake failed: " << error.message());
            }

            CallHandler(endpoint, Flags::error);
            return;
          }
          CallHandler(endpoint, Flags::good);

          context->socket.async_shutdown(
              [this, _ = shared_from_this(),
               context](const boost::system::error_code&) {});
        });
  };

  boost::asio::async_connect(socket.lowest_layer(), endpoints_,
                             std::move(connect_handler));
}

void CertificateCheckWorker::Start() {
  LOG_INFO(<< host_name_);
  for (auto& endpoint : endpoints_) {
    CheckEndpoint(endpoint);
  }
}
void CertificateCheckWorker::CallHandler(
    const boost::asio::ip::tcp::endpoint& endpoint, uintptr_t flags) {
  auto countdown = --callback_countdown_;
  if (countdown == 0) {
    flags |= Flags::finished;
  }
  handler_(host_name_, endpoint, flags);
}

}  // namespace tls
}  // namespace hood_proxy
