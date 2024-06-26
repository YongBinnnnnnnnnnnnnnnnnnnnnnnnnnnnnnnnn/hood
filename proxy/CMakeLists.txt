cmake_minimum_required(VERSION 3.1.0)
project (hood_proxy)


set(Boost_NO_WARN_NEW_VERSIONS 1)
set(Boost_USE_STATIC_LIBS ON)
find_package( Boost 1.84 COMPONENTS program_options system log REQUIRED )
set(OPENSSL_USE_STATIC_LIBS ON)
find_package( OpenSSL REQUIRED )
include_directories (${Boost_INCLUDE_DIR} ${OPENSSL_INCLUDE_DIR} SYSTEM)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Get version from version.h
file(READ src/version.h version_h)
if (NOT version_h MATCHES "HOOD_PROXY_VERSION ([0-9]+)([0-9][0-9])([0-9][0-9])")
  message(FATAL_ERROR "Cannot get HOOD_PROXY_VERSION from version.h.")
endif ()

# Use math to skip leading zeros if any.
math(EXPR CPACK_PACKAGE_VERSION_MAJOR ${CMAKE_MATCH_1})
math(EXPR CPACK_PACKAGE_VERSION_MINOR ${CMAKE_MATCH_2})
math(EXPR CPACK_PACKAGE_VERSION_PATCH ${CMAKE_MATCH_3})
 
include_directories("${CMAKE_CURRENT_SOURCE_DIR}")
 
# add the executable
add_executable(hood_proxy 
  src/main.cpp src/configuration.cpp src/engine.cpp src/server.cpp src/logging.cpp src/tls_message_decoder.cpp src/tls_context.cpp
  src/network.cpp src/tls_security_policy.cpp src/tls_check_certificate.cpp src/tls_check_certificate_worker.cpp
  src/name_service_client.cpp
)
set_property(TARGET hood_proxy PROPERTY POSITION_INDEPENDENT_CODE TRUE)

if(MSVC)
  target_compile_options(hood_proxy PRIVATE /W4 /WX)
else()
  target_compile_options(hood_proxy PRIVATE -Wall -Wextra -Werror -fstack-protector)
endif()


target_link_libraries( hood_proxy ${Boost_LIBRARIES} pthread ${OPENSSL_LIBRARIES} )
