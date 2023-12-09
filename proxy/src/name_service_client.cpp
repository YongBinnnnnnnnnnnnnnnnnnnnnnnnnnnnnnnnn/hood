#include <boost/algorithm/string/predicate.hpp>
#include <boost/asio.hpp>
#include <cstring>

#include "configuration.hpp"
#include "engine.hpp"
#include "logging.hpp"
#include "name_service_client.hpp"

using boost::algorithm::ends_with;
using boost::asio::async_read;
using boost::asio::async_write;
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
    offset += length;
  }
}

void Resolve(const std::string& host_name, ResultHandler&& handler) {
  struct Context {
    Context(ResultHandler&& result_handler)
        : socket(hood_proxy::Engine::get().GetExecutor()),
          handler(result_handler) {}
    udp::socket socket;
    std::array<uint8_t, 65536> buffer;
    ResultHandler handler;
    udp::endpoint response_sender;
  };
  auto host_name_length = host_name.length();
  if (host_name_length > hood_proxy::Configuration::max_length_of_a_domain) {
    auto error = boost::system::errc::make_error_code(
        boost::system::errc::invalid_argument);
    handler(error, dummy_addresses_);
    return;
  }
  auto context = std::unique_ptr<Context>(
      new Context(std::move(handler)));  // std::make_unique<Context>();
  auto& socket = context->socket;
  auto& buffer = context->buffer;
  memcpy(buffer.data(), host_name.data(), host_name_length);
  auto send_result_handler = [context_raw_pointer = context.release()](
                                 const boost::system::error_code& error,
                                 std::size_t /*bytes_transfered*/) {
    auto context = std::unique_ptr<Context>(context_raw_pointer);
    if (error) {
      context->handler(error, dummy_addresses_);
      return;
    }

    auto& socket = context->socket;
    auto& buffer = context->buffer;
    auto& response_sender = context->response_sender;
    auto receive_result_handler = [context_raw_pointer = context.release()](
                                      const boost::system::error_code& error,
                                      std::size_t bytes_transfered) {
      auto context = std::unique_ptr<Context>(context_raw_pointer);
      if (error) {
        context->handler(error, dummy_addresses_);
        return;
      }
      std::vector<boost::asio::ip::address> result;
      DecodeResult(result, context->buffer.data(), bytes_transfered);
      context->handler(error, result);
    };
    socket.async_receive_from(boost::asio::buffer(buffer.data(), buffer.size()),
                              response_sender,
                              std::move(receive_result_handler));
  };
  socket.open(udp::v4());
  socket.async_send_to(boost::asio::buffer(buffer.data(), host_name_length),
                       service_endpont_, std::move(send_result_handler));
}

}  // namespace name_service
}  // namespace hood
