#include <boost/algorithm/string/predicate.hpp>
#include <boost/asio.hpp>
#include <chrono>
#include <cstring>

#include "configuration.hpp"
#include "engine.hpp"
#include "logging.hpp"
#include "name_service_client.hpp"

using boost::algorithm::ends_with;
using boost::asio::async_read;
using boost::asio::async_write;
using boost::asio::steady_timer;
using boost::asio::ip::make_address;
using boost::asio::ip::udp;
using boost::system::error_code;

namespace hood {
namespace name_service {

static auto service_endpont_ = udp::endpoint(make_address("127.0.0.1"), 530);
static std::vector<boost::asio::ip::address> dummy_addresses_;

static void DecodeResult(std::vector<boost::asio::ip::address>& output,
                         const uint8_t* buffer, size_t size) {
  size_t offset = 0;
  while (offset < size) {
    auto start = reinterpret_cast<const char*>(buffer + offset);
    auto length = strnlen(start, size - offset);
    std::string_view entry(start, length);
    boost::system::error_code error;
    auto address = make_address(entry, error);
    if (!error) {
      output.emplace_back(std::move(address));
    }
    offset += length + 1;
  }
}


class Client: public std::enable_shared_from_this<Client> {
public:
  using pointer = std::shared_ptr<Client>;

  static pointer create() {
    return pointer(new Client());
  }
  void Start(const std::string& host_name, ResolveResultHandler&& the_handler);
private:
  constexpr static auto timeout_ = std::chrono::seconds(5);
  Client()
        : socket_(hood_proxy::Engine::get().GetExecutor()),
          timer_(hood_proxy::Engine::get().GetExecutor()),
          attempts_(0) {}
  udp::socket socket_;
  std::array<uint8_t, 65536> buffer_;
  size_t data_size_;
  ResolveResultHandler handler_;
  udp::endpoint response_sender_;
  steady_timer timer_;
  uintptr_t attempts_;
  
  void Attempt();
};

void Client::Start(const std::string& host_name, ResolveResultHandler&& handler) {
  data_size_ = host_name.length();
  if (data_size_ > hood_proxy::Configuration::max_length_of_a_domain) {
    auto error = boost::system::errc::make_error_code(
        boost::system::errc::invalid_argument);
    handler(error, dummy_addresses_);
    return;
  }
  memcpy(buffer_.data(), host_name.data(), data_size_);
  handler_ = std::move(handler);
  Attempt();
}

void Client::Attempt() {
  attempts_++;
  if (attempts_ > 3) {
    auto error = boost::system::errc::make_error_code(
        boost::system::errc::timed_out);
    handler_(error, dummy_addresses_);
    return;
  }
  if (attempts_ > 1) {
    boost::system::error_code error;
    socket_.shutdown(udp::socket::shutdown_both, error);
    socket_.cancel(error);
    socket_.close();
    socket_ = udp::socket(hood_proxy::Engine::get().GetExecutor());
  }
  
  auto send_result_handler = [this, _ = shared_from_this()](
                                 const boost::system::error_code& error,
                                 std::size_t /*bytes_transfered*/) {
    if (error) {
      handler_(error, dummy_addresses_);
      return;
    }

    auto receive_result_handler = [this, _ = shared_from_this()](
                                      const boost::system::error_code& error,
                                      std::size_t bytes_transfered) {
      timer_.cancel();
      if (error) {
        handler_(error, dummy_addresses_);
        return;
      }
      std::vector<boost::asio::ip::address> result;
      DecodeResult(result, buffer_.data(), bytes_transfered);
      handler_(error, result);
    };
    
    timer_.expires_after(timeout_);
    timer_.async_wait(
      [this, _ = shared_from_this()](const boost::system::error_code& error){
        if (error != boost::asio::error::operation_aborted) {
          socket_.cancel();
          Attempt();
        }        
      });
      
    socket_.async_receive_from(boost::asio::buffer(buffer_.data(), buffer_.size()),
                              response_sender_, std::move(receive_result_handler));
    
  };
  socket_.open(udp::v4());
  socket_.async_send_to(boost::asio::buffer(buffer_.data(), data_size_),
                       service_endpont_, std::move(send_result_handler));
}

void Resolve(const std::string& host_name, ResolveResultHandler&& handler) {
  Client::create()->Start(host_name, std::move(handler));
}

}  // namespace name_service
}  // namespace hood
