#pragma once

#include "../utils/utils.h"

class session_mgr {
public:
  session_mgr(io_context &ioc);
  ~session_mgr();

  uint64 create_session(tcp::socket &socket, std::string &proto_name);
  uint64 create_session(std::string &proto_name, std::string &host,
                        std::string &port);
  void remove_session(uint64 id);

  bool session_valid(uint64 session_id);
  void send_msg(uint64 id, std::string &msg);

private:
  struct impl;
  impl *impl_ptr;
};

extern std::shared_ptr<session_mgr> g_session_mgr;