#ifndef HOOD_PROXY_ENGINE_H_
#define HOOD_PROXY_ENGINE_H_
#include <boost/asio.hpp>

namespace hood_proxy {

// TODO: support seastar

class Engine {
 public:
  static inline Engine& get() {
    static thread_local Engine object;
    return object;
  }
  static inline void DebugBreak() {
#ifndef NDEBUG
    __builtin_trap();
#endif
  }

  inline boost::asio::io_context& GetExecutor() { return raw_object_; }

 private:
  boost::asio::io_context raw_object_;
  Engine();
};

}  // namespace hood_proxy
#endif  // HOOD_PROXY_ENGINE_H_