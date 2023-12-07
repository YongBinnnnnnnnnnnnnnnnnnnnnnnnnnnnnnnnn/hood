#include <boost/log/expressions.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <iostream>

#include "logging.hpp"
namespace hood_proxy {

namespace expr = boost::log::expressions;

#ifndef __GNUC__
#define BUGGY_GCC 0
#else
#define BUGGY_GCC \
  (__GNUC__ == 12 && __GNUC_MINOR__ == 2 && __GNUC_PATCHLEVEL__ == 0)
#endif

#if !BUGGY_GCC

const auto date_time_formatter =
    expr::stream << expr::format_date_time<boost::posix_time::ptime>(
                        "TimeStamp", "\e[32m%m%d %H:%M:%S.%f")
                 << expr::message;
#endif

const auto simple_formatter = expr::stream << expr::message;
void InitLogging() {
  boost::log::add_common_attributes();
#if BUGGY_GCC
  boost::log::add_console_log();
#else
  boost::log::add_console_log()->set_formatter(date_time_formatter);
#endif
}

}  // namespace hood_proxy