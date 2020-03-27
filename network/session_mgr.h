#pragma once

#include "../utils/utils.h"

class session_mgr {
public:
  session_mgr(io_context &ioc);
  ~session_mgr();

  uint64 create_session(tcp::socket &socket, const std::string &proto_name);
  uint64 connect_to(const std::string &proto_name, const std::string &host,
                    const std::string &port);
  void remove_session(uint64 id);

  bool session_valid(uint64 session_id);
  void send_msg(uint64 id, const std::string &msg);

private:
  struct impl;
  impl *impl_ptr;
};

extern std::shared_ptr<session_mgr> g_session_mgr;