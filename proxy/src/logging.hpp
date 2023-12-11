#ifndef HOOD_PROXY_LOGGING_H_
#define HOOD_PROXY_LOGGING_H_

#include <boost/log/trivial.hpp>
#include <cassert>

namespace hood_proxy {
void InitLogging();
}  // namespace hood_proxy

#ifdef NDEBUG

#ifndef __FILE_NAME__
#define __FILE_NAME__ __FILE__
#endif  // __FILE_NAME__

#define HOOD_PROXY_LOG_TRIVIAL2(severity, color, line) \
  BOOST_LOG_TRIVIAL(severity)                          \
      << color __FILE_NAME__ ":" #line " " << __FUNCTION__ << ": "

#else  // NDEBUG
#define HOOD_PROXY_LOG_TRIVIAL2(severity, color, line) \
  BOOST_LOG_TRIVIAL(severity)                          \
      << color __FILE__ ":" #line " " << __FUNCTION__ << ": "
#endif  // NDEBUG

#define HOOD_PROXY_LOG_TRIVIAL(severity, color, line) \
  HOOD_PROXY_LOG_TRIVIAL2(severity, color, line)

#ifdef NDEBUG
#define LOG_TRACE(expression)
#define LOG_DEBUG(expression)
#else
#define LOG_TRACE(expression) \
  HOOD_PROXY_LOG_TRIVIAL(trace, "\e[90m", __LINE__) expression
#define LOG_DEBUG(expression) \
  HOOD_PROXY_LOG_TRIVIAL(debug, "\e[37m", __LINE__) expression
#endif

#define LOG_INFO(expression) \
  HOOD_PROXY_LOG_TRIVIAL(info, "\e[39m", __LINE__) expression
#define LOG_ERROR(expression) \
  HOOD_PROXY_LOG_TRIVIAL(error, "\e[91m", __LINE__) expression

#endif  // HOOD_PROXY_LOGGING_H_
