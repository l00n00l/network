#pragma once

#include "../utils/utils.h"

class tcp_server {
public:
  tcp_server(io_context &ioc, tcp::endpoint endpoint, std::string &proto_name);
  ~tcp_server();

private:
  void _do_accept();

private:
  struct impl;
  impl *impl_ptr;
};
