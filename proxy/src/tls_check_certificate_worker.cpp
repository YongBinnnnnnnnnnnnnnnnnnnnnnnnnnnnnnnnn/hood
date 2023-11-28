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

struct CertificateVerificationContext {
  using stream_type = boost::asio::ssl::stream<boost::asio::ip::tcp::socket>;
  boost::asio::ssl::context ssl_context;
  SSL_SESSION* ssl_session = nullptr;
  stream_type socket;
  void Close() {
    if (!socket.lowest_layer().is_open()) {
      return;
    }

    ssl_session = SSL_get1_session(socket.native_handle());
    socket.lowest_layer().cancel();
    socket.lowest_layer().close();
  }
  CertificateVerificationContext(boost::asio::ssl::context&& ssl_context)
      : ssl_context(ssl::context::tls_client),
        socket(Engine::get().GetExecutor(), ssl_context) {}
  ~CertificateVerificationContext() {
    Close();
    SSL_SESSION_free(ssl_session);
  }
};

CertificateCheckWorker::CertificateCheckWorker(
    const std::string& host_name, const tcp_endpoints_type& endpoints,
    ResultHandler handler)
    : host_name_(host_name),
      endpoints_(endpoints),
      callback_countdown_(endpoints.size()),
      handler_(std::move(handler)) {}

static inline boost::asio::ssl::context BuildSSLContext() {
  // TODO: support more tls option from configuration
  // Use system cert
  boost::asio::ssl::context result(ssl::context::tls_client);
  result.set_default_verify_paths();
  // lowest tls version set to 1.2
  result.set_options(ssl::context::default_workarounds |
                     ssl::context::no_tlsv1 | ssl::context::no_tlsv1_1);
  SSL_CTX_set_session_cache_mode(result.native_handle(),
                                 SSL_SESS_CACHE_NO_INTERNAL);
  return result;
}

void CertificateCheckWorker::CheckEndpoint(
    boost::asio::ip::tcp::endpoint& endpoint) {
  LOG_INFO(<< host_name_);
  auto context =
      std::make_unique<CertificateVerificationContext>(BuildSSLContext());

  auto& socket = context->socket;

  socket.set_verify_mode(ssl::verify_peer);

  {
    // Enable automatic hostname checks
    auto param = SSL_get0_param(socket.native_handle());

    X509_VERIFY_PARAM_set_hostflags(param,
                                    X509_CHECK_FLAG_NO_PARTIAL_WILDCARDS);
    X509_VERIFY_PARAM_set1_host(param, host_name_.data(), host_name_.length());
  }

  {
    // Add Server Name Indication SNI
    SSL_set_tlsext_host_name(socket.native_handle(), host_name_.data());
  }

  socket.set_verify_callback(
      [host_name = host_name_](bool preverified,
                               boost::asio::ssl::verify_context& ctx) {
        char subject_name[256];
        X509* cert = X509_STORE_CTX_get_current_cert(ctx.native_handle());
        X509_NAME_oneline(X509_get_subject_name(cert), subject_name, 256);
        LOG_INFO(" verifying " << host_name << " :" << subject_name);
        return ssl::host_name_verification(host_name)(preverified, ctx);
      });

  auto connect_handler = [this, _ = shared_from_this(),
                          context = context.release(),
                          &endpoint](const boost::system::error_code& error,
                                     const tcp::endpoint& /*endpoint*/) {
    std::unique_ptr<CertificateVerificationContext> context_unque_ptr(context);
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
        [this, _ = shared_from_this(), context = context_unque_ptr.release(),
         &endpoint](const boost::system::error_code& error) {
          std::unique_ptr<CertificateVerificationContext> context_unque_ptr(
              context);
          if (!context->socket.lowest_layer().is_open()) {
            CallHandler(endpoint, Flags::error);
            return;
          }
          if (error) {
            LOG_ERROR(<< host_name_
                      << " handshake failed: " << error.message());
            CallHandler(endpoint, Flags::error);
            return;
          }
          CallHandler(endpoint, Flags::good);
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
