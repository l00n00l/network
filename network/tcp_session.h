#pragma once

#include "../utils/utils.h"

class tcp_session {
public:
  tcp_session(uint64 id, tcp::socket &socket, const std::string &proto_name);
  tcp_session(uint64 id, io_context &ioc, const std::string &proto_name,
              tcp_resolve_result endpoints);
  ~tcp_session();
  bool valid();
  void send(const std::string &msg);

private:
  void _do_read();
  void _do_write();
  void _do_disconnect();
  void _do_connect();
  void _set_reconnect_timer();

private:
  struct impl;
  impl *impl_ptr;
};
