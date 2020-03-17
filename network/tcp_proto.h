#pragma once
#include "../utils/utils.h"

struct tcp_proto {
  tcp_proto(const std::string &proto_name, const std::string &side);
  ~tcp_proto();
  uint8 read_type();
  char *read_buffer_ptr();
  std::string &dbuffer();
  asio_stream_buf &sbuffer();
  const char *write_buffer_ptr();
  std::size_t read_buffer_size();
  regex &until_regex();
  std::size_t write_buffer_size();
  void read_done(std::size_t size, uint64 session_id);
  void write_done(std::size_t size);
  bool has_data_to_write();
  void write(const std::string &msg);

private:
private:
  struct impl;
  impl *impl_ptr;
};

std::unique_ptr<tcp_proto> create_proto(const std::string &proto_name,
                                        const std::string &server_or_client);
bool load_protos(const std::string &path);
typedef void (*message_handler_type)(uint64 session_id, uint64 data_id,
                                     const char *proto_name);