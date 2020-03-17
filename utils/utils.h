#pragma once

#include "log.h"
#include <atomic>
#include <boost/asio.hpp>
#include <boost/filesystem.hpp>
#include <boost/locale.hpp>
#include <boost/multiprecision/cpp_bin_float.hpp>
#include <boost/multiprecision/cpp_complex.hpp>
#include <boost/multiprecision/cpp_int.hpp>
#include <boost/regex.hpp>
#include <boost/thread.hpp>
#include <cstdint>
#include <sstream>
#include <string>
using boost::locale::conv::from_utf;
using boost::locale::conv::to_utf;
using boost::locale::conv::utf_to_utf;

namespace fsys = boost::filesystem;

typedef boost::match_results<std::string::const_iterator> match_results_str;
using boost::regex;
using boost::regex_match;
using boost::regex_search;

using boost::asio::async_connect;
using boost::asio::async_read;
using boost::asio::async_read_at;
using boost::asio::async_read_until;
using boost::asio::async_write;
using boost::asio::bind_executor;
using boost::asio::buffer;
using boost::asio::buffers_begin;
using boost::asio::buffers_end;
using boost::asio::dynamic_buffer;
using boost::asio::io_context;
using boost::asio::steady_timer;
using asio_stream_buf = boost::asio::streambuf;
using boost::asio::use_future;
using boost::asio::ip::address;
using boost::asio::ip::tcp;
using boost::asio::ip::udp;
using boost::system::error_code;

typedef boost::asio::ip::basic_resolver_results<tcp> tcp_resolve_result;
typedef boost::asio::strand<boost::asio::executor> exe_strand;

#define atomic_flag_acquire(flag)                                              \
  while (flag.test_and_set(std::memory_order_acquire))                         \
  boost::this_thread::yield()
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
