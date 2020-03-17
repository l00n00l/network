#pragma once
#include "log.h"
log_func log_info_func = nullptr;
log_func log_debug_func = nullptr;
log_func log_error_func = nullptr;

void log_info(const std::string &msg) {
  if (log_info_func)
    log_info_func(msg.c_str(), msg.size());
}

void log_debug(const std::string &msg) {
  if (log_debug_func)
    log_debug_func(msg.c_str(), msg.size());
}

void log_error(const std::string &msg) {
  if (log_error_func)
    log_error_func(msg.c_str(), msg.size());
}

void set_info_log_func(log_func func_ptr) { log_info_func = func_ptr; }

void set_debug_log_func(log_func func_ptr) { log_debug_func = func_ptr; }

void set_error_log_func(log_func func_ptr) { log_error_func = func_ptr; }

log_stream lsinfo(LOG_INFO);
log_stream lsdebug(LOG_DEBUG);
log_stream lserr(LOG_ERROR);