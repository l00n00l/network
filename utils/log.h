#pragma once
#include <boost/lexical_cast.hpp>
#include <cstdint>
#include <string>
void log_info(const std::string &msg);
void log_debug(const std::string &msg);
void log_error(const std::string &msg);

typedef void (*log_func)(const char *, std::size_t);

void set_info_log_func(log_func func_ptr);
void set_debug_log_func(log_func func_ptr);
void set_error_log_func(log_func func_ptr);

enum { LOG_INFO = 0, LOG_DEBUG = 1, LOG_ERROR = 2 };

class log_stream {
public:
  log_stream(uint8_t type) : log_type(type){};
  log_stream(log_stream &) = delete;
  log_stream(log_stream &&) = delete;
  ~log_stream() = default;
  template <typename T> log_stream &operator<<(T &value) {
    try {
      ss << value;
    } catch (const std::exception &e) {
      log_error(e.what());
    }
    return *this;
  }
  template <typename T> log_stream &operator>>(T &value) {
    try {
      ss << value;
      switch (log_type) {
      case LOG_INFO:
        log_info(ss.str());
        break;
      case LOG_DEBUG:
        log_debug(ss.str());
        break;
      case LOG_ERROR:
        log_error(ss.str());
        break;
      }
      ss.str("");
    } catch (const std::exception &e) {
      log_error(e.what());
    }
    return *this;
  }

private:
  uint8_t log_type;
  std::stringstream ss;
};

extern log_stream lsinfo;
extern log_stream lsdebug;
extern log_stream lserr;
