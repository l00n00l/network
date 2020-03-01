#pragma once
#include "../utils/utils.h"
#include <iostream>

struct tcp_proto {
  virtual char *read_buffer_ptr() = 0;
  virtual const char *write_buffer_ptr() = 0;
  virtual std::size_t read_buffer_size() = 0;
  virtual std::size_t write_buffer_size() = 0;
  virtual void read_done(std::size_t size, uint64 session_id) = 0;
  virtual void write_done(std::size_t size) = 0;
  virtual bool has_data_to_write() = 0;
  virtual void write(std::string &msg) = 0;
};

std::unique_ptr<tcp_proto> create_proto(std::string &proto_name);

typedef void (*control_handler)(char *data, std::size_t size,
                                uint64 session_id);
void regist_contol_handler(control_handler handler);