#pragma once

#include <atomic>
#include <boost/asio.hpp>
#include <boost/multiprecision/cpp_bin_float.hpp>
#include <boost/multiprecision/cpp_complex.hpp>
#include <boost/multiprecision/cpp_int.hpp>
#include <cstdint>
#include <string>

using boost::asio::async_connect;
using boost::asio::async_read;
using boost::asio::async_write;
using boost::asio::bind_executor;
using boost::asio::buffer;
using boost::asio::dynamic_buffer;
using boost::asio::io_context;
using boost::asio::steady_timer;
using boost::asio::use_future;
using boost::asio::ip::address;
using boost::asio::ip::tcp;
using boost::asio::ip::udp;
using boost::system::error_code;

typedef boost::asio::ip::basic_resolver_results<tcp> tcp_resolve_result;
typedef boost::asio::strand<boost::asio::executor> exe_strand;

#define atomic_flag_acquire(flag)                                              \
  while (flag.test_and_set(std::memory_order_acquire))
#define atomic_flag_release(flag) flag.clear(std::memory_order_release)

using int8 = int8_t;
using int16 = int16_t;
using int32 = int32_t;
using int64 = int64_t;
using uint8 = uint8_t;
using uint16 = uint16_t;
using uint32 = uint32_t;
using uint64 = uint64_t;
using boost::multiprecision::cpp_int;

using float32 = float;
using float64 = double;
using float128 = boost::multiprecision::cpp_bin_float<128>;
using boost::multiprecision::cpp_bin_float;
