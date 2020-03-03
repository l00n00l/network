#pragma once

#include "../utils/utils.h"

class server_mgr {
public:
  server_mgr(io_context &ioc);
  ~server_mgr();

  void create_server(tcp::endpoint endpoint, const std::string &proto_name);
  void destroy_server(uint64 server_id);

private:
  struct impl;
  impl *impl_ptr;
};

extern std::shared_ptr<server_mgr> g_server_mgr;