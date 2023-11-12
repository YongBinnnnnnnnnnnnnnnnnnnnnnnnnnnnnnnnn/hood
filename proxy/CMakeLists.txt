cmake_minimum_required(VERSION 3.1.0)
project (hood_proxy)


set(Boost_NO_WARN_NEW_VERSIONS 1)
set(BOOST_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/libs/${CMAKE_HOST_SYSTEM_NAME}/boost_1_83_0-install")
set(Boost_USE_STATIC_LIBS ON)
find_package( Boost 1.83 COMPONENTS program_options system log REQUIRED )
include_directories (SYSTEM ${Boost_INCLUDE_DIR})

set(CMAKE_CXX_STANDARD 17)
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
  src/main.cpp src/configuration.cpp src/engine.cpp src/server.cpp src/logging.cpp src/tls_message_decoder.cpp src/tls_message_encoder.cpp src/tls_context.cpp
)

if(MSVC)
  target_compile_options(hood_proxy PRIVATE /W4 /WX)
else()
  target_compile_options(hood_proxy PRIVATE -Wall -Wextra -Werror)
endif()

target_link_libraries( hood_proxy ${Boost_LIBRARIES} pthread ssl crypto )
