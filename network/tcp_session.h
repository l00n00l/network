#pragma once

#include "../utils/utils.h"

class tcp_session {
public:
  tcp_session(uint64 id, tcp::socket &socket, std::string &proto_name);
  tcp_session(uint64 id, io_context &ioc, std::string &proto_name,
              std::string &host, std::string &name);
  ~tcp_session();
  bool valid();
  void send(std::string &msg);

private:
  void _do_read();
  void _do_write();
  void _do_disconnect();

private:
  struct impl;
  impl *impl_ptr;
};
