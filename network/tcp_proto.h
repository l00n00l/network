#pragma once
#include "../utils/utils.h"

enum side_index { side_server, side_client };
enum read_type { none, read_some, read_until, read_size, read_var_size };

struct tcp_proto {
  tcp_proto(const std::string &proto_name, side_index side);
  ~tcp_proto();
  read_type cur_read_type();
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
  struct impl;
  impl *impl_ptr;
};

typedef void (*message_handler_type)(uint64 session_id, uint64 data_id,
                                     const char *proto_name);
void set_message_handler(message_handler_type handler);

std::unique_ptr<tcp_proto> create_proto_server(const std::string &proto_name);
std::unique_ptr<tcp_proto> create_proto_client(const std::string &proto_name);

bool load_protos(const std::string &path);

std::size_t get_read_item_size(const std::string &proto_name, std::size_t side);

const std::string get_compile_regex(const std::string &proto_name,
                                    std::size_t side, std::size_t index);
