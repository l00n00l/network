#pragma once

#include "log.h"
#include <atomic>
#include <boost/asio.hpp>
#include <boost/filesystem.hpp>
#include <boost/locale.hpp>
#include <boost/multiprecision/cpp_bin_float.hpp>
#include <boost/multiprecision/cpp_complex.hpp>
#include <boost/multiprecision/cpp_dec_float.hpp>
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
using boost::multiprecision::cpp_bin_float;
using boost::multiprecision::cpp_int;
using boost::multiprecision::number;
using float32 = float;
using float64 = double;
using float128 = boost::multiprecision::cpp_bin_float_quad;

#define TOTRY try {
#define TOCATCH                                                                \
  }                                                                            \
  catch (const boost::bad_lexical_cast &e) {                                   \
    lserr << __FUNCTION__ >> ": data = " << data << " exception:" >>           \
        std::string(e.what());                                                 \
    return 0;                                                                  \
  }

inline int8 to_int8(const std::string &data) {
  TOTRY
  return boost::lexical_cast<int8>(data);
  TOCATCH
}
inline int16 to_int16(const std::string &data) {
  TOTRY
  return boost::lexical_cast<int16>(data);
  TOCATCH
}
inline int32 to_int32(const std::string &data) {
  TOTRY
  return boost::lexical_cast<int32>(data);
  TOCATCH
}
inline int64 to_int64(const std::string &data) {
  TOTRY
  return boost::lexical_cast<int64>(data);
  TOCATCH
}
inline uint8 to_uint8(const std::string &data) {
  TOTRY
  return boost::lexical_cast<uint8>(data);
  TOCATCH
}
inline uint16 to_uint16(const std::string &data) {
  TOTRY
  return boost::lexical_cast<uint16>(data);
  TOCATCH
}
inline uint32 to_uint32(const std::string &data) {
  TOTRY
  return boost::lexical_cast<uint32>(data);
  TOCATCH
}
inline uint64 to_uint64(const std::string &data) {
  TOTRY
  return boost::lexical_cast<uint64>(data);
  TOCATCH
}
inline float32 to_float32(const std::string &data) {
  TOTRY
  return boost::lexical_cast<float32>(data);
  TOCATCH
}
inline float64 to_float64(const std::string &data) {
  TOTRY
  return boost::lexical_cast<float64>(data);
  TOCATCH
}
inline float128 to_float128(const std::string &data) {
  TOTRY
  return float128(data);
  TOCATCH
}

inline void strcpy(char *des, const char *src, std::size_t size) {
  for (std::size_t i = 0; i < size; i++) {
    des[i] = src[i];
  }
}

inline std::string sbuffer_string(asio_stream_buf &buf, std::size_t size = 0) {
  if (size == 0)
    size = buf.size();
  auto begin = buffers_begin(buf.data());
  auto ret = std::string(begin, begin + size);
  buf.consume(size);
  return ret;
}