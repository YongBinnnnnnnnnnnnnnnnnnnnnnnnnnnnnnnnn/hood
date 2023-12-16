#ifndef HOOD_PROXY_MESSAGE_READER_H_
#define HOOD_PROXY_MESSAGE_READER_H_
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/endian/conversion.hpp>
#include <cassert>
#include <string>
#include <type_traits>
#include <vector>

#include "logging.hpp"

namespace hood_proxy {

template <typename T>
static constexpr size_t MaxValueOfType(size_t current = 256, size_t iteration = sizeof(T)) {
  static_assert(std::is_integral_v<T>);
  if (iteration == 1) {    
    if (std::is_signed_v<T>) {
      return current / 2 - 1;
    }
    return current -1;
  }
  return MaxValueOfType<T>(current * 256, iteration - 1);
}
static_assert(MaxValueOfType<int16_t>() == 32767);
static_assert(MaxValueOfType<uint16_t>() == 65535);

template <typename RawMessageType>
class MessageReader {
 public:
  enum class Reason {
    NEW_MESSAGE,
    IO_ERROR,
    MANUALLY_STOPPED,
    CONNECTION_CLOSED,
  };

  enum class NextStep {
    Read,
    Pause,
    Stop,
  };

  using StreamHandlerTypeExample =
      std::function<NextStep(Reason, const uint8_t*, uint16_t)>;
  using UdpHandlerTypeExample = std::function<NextStep(
      Reason, const uint8_t*, uint16_t, boost::asio::ip::udp::endpoint*)>;

  void resize_buffer(size_t size) { buffer_.resize(size); }

  void reset() {
    status_ = Status::STOP;
    data_offset_ = 0;
    data_size_ = 0;
    tcp_message_size_ = 0;
  }

  template <typename StreamType, typename HandlerType>
  void Start(std::shared_ptr<StreamType> stream, HandlerType&& handler) {
    if (status_ != Status::RUNNING) {
      status_ = Status::RUNNING;
      if constexpr (std::is_same<StreamType,
                                 boost::asio::ip::udp::socket>::value) {
        if (buffer_.size() == 0) {
          assert(false);
          LOG_ERROR("Reading udp to empty buffer");
          return;
        }
        DoReadUdp(stream, std::move(handler));
      } else {
        DoReadStream(stream, std::move(handler));
      }
    }
  }

  void Stop() { status_ = Status::STOP; }

 private:
  using TcpSocket = boost::asio::ip::tcp::socket;
  using UdpSocket = boost::asio::ip::udp::socket;

  enum class Status { STOP, RUNNING } status_ = Status::STOP;
  size_t data_offset_ = 0;
  size_t data_size_ = 0;
  size_t tcp_message_size_ = 0;
  boost::asio::ip::udp::endpoint udp_endpoint_;

  std::vector<uint8_t> buffer_;

  template <typename StreamPointerType, typename HandlerType>
  void DoReadStream(StreamPointerType stream_pointer, HandlerType&& handler) {
    if (status_ == Status::STOP) {
      handler(Reason::MANUALLY_STOPPED, nullptr, 0);
      status_ = Status::STOP;
      LOG_TRACE("manual stopped");
      return;
    }
    if (!stream_pointer->lowest_layer().is_open()) {
      handler(Reason::CONNECTION_CLOSED, nullptr, 0);
      status_ = Status::STOP;
      LOG_TRACE("connection closed");
      return;
    }

    do {
      auto data = buffer_.data() + data_offset_;
      if (tcp_message_size_ == 0) {
        if (data_size_ < sizeof(RawMessageType)) {
          break;
        }
        auto tcp_message = reinterpret_cast<const RawMessageType*>(data);
        tcp_message_size_ =
            offsetof(RawMessageType, message_length) +
            sizeof(RawMessageType::message_length) +
            boost::endian::big_to_native(tcp_message->message_length);
      }
      LOG_TRACE("Message " << this << " " << data_size_ << "/" << tcp_message_size_);
      if (data_size_ < tcp_message_size_) {
        break;
      }
      auto next = handler(Reason::NEW_MESSAGE, data, tcp_message_size_);
      data_size_ -= tcp_message_size_;
      data_offset_ += tcp_message_size_;
      tcp_message_size_ = 0;

      if (next == NextStep::Pause) {
        return;
      } else if (next == NextStep::Stop) {
        Stop();
        return;
      }
    } while (data_size_ >= tcp_message_size_);

    if (data_size_ == 0) {
      data_offset_ = 0;
    }
    auto available_size = buffer_.size() - data_offset_ - data_size_;
    size_t read_size = sizeof(RawMessageType);;
    
    if (tcp_message_size_) {
      read_size = tcp_message_size_ - data_size_;
    }
    assert(read_size < MaxValueOfType<decltype(RawMessageType::message_length)>());
    if (available_size < read_size) {
      if (data_offset_ + available_size > read_size) {
        memmove(buffer_.data(), buffer_.data() + data_offset_, data_size_);
        available_size += data_offset_;
        data_offset_ = 0;
      } else {
        buffer_.resize(buffer_.size() + (read_size - available_size));
        available_size = read_size;
      }
    }

    auto read_buffer = buffer_.data() + data_offset_ + data_size_;

    LOG_TRACE("start async_read " << read_size);

    auto raw_stream_pointer = &*stream_pointer;

    auto boost_handler = [this, stream_pointer = std::move(stream_pointer),
                          handler = std::move(handler)](
                             boost::system::error_code error,
                             size_t new_data_size) {
      if (error) {
        if (error == boost::asio::error::eof ||
            error == boost::system::errc::operation_canceled) {
          LOG_TRACE("connection closed");
          // TODO: add a handler reason for this
          status_ = Status::STOP;
          return;
        }
        LOG_ERROR(<< error.message());
        handler(Reason::IO_ERROR, nullptr, 0);
        status_ = Status::STOP;
        return;
      }

      LOG_TRACE("Income data " << new_data_size);
      data_size_ += new_data_size;

      DoReadStream(stream_pointer, std::move(handler));
    };

    boost::asio::async_read(
        *raw_stream_pointer, boost::asio::buffer(read_buffer, available_size),
        boost::asio::transfer_at_least(read_size), boost_handler);
  }

  template <typename HandlerType>
  void DoReadUdp(boost::asio::ip::udp::socket& socket, HandlerType&& handler) {
    if (status_ == Status::STOP) {
      handler(Reason::MANUALLY_STOPPED, nullptr, 0, nullptr);
      status_ = Status::STOP;
      LOG_TRACE("manual stopped");
      return;
    }
    if (!socket.is_open()) {
      handler(Reason::MANUALLY_STOPPED, nullptr, 0, nullptr);
      status_ = Status::STOP;
      LOG_TRACE("connection closed");
      return;
    }
    socket.async_receive_from(
        boost::asio::buffer(buffer_), udp_endpoint_,
        [this, &socket, handler = std::move(handler)](
            boost::system::error_code error, size_t data_size) {
          if (error) {
            if (error == boost::asio::error::eof ||
                error == boost::system::errc::operation_canceled) {
              LOG_TRACE("connection closed");
              // TODO: add a handler reason for this
              status_ = Status::STOP;
              return;
            }
            LOG_ERROR(<< error.message());
            handler(Reason::IO_ERROR, nullptr, 0, nullptr);
            status_ = Status::STOP;
            return;
          }
          auto next = handler(Reason::NEW_MESSAGE, buffer_.data(), data_size,
                              &udp_endpoint_);
          if (next == NextStep::Read) {
            DoReadUdp(socket, std::move(handler));
          } else if (next == NextStep::Stop) {
            Stop();
          }
        });
  }
};

}  // namespace hood_proxy
#endif  // HOOD_PROXY_MESSAGE_READER_H_
