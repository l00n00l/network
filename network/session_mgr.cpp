#include "session_mgr.h"
#include "../utils/id_generator.h"
#include "session_mgr.h"
#include "tcp_session.h"
#include <unordered_map>

std::shared_ptr<session_mgr> g_session_mgr;

typedef std::unordered_map<uint64, std::shared_ptr<tcp_session>>
    tcp_session_map;
struct session_mgr::impl {
  id_generator id_gen;
  tcp_session_map tcp_sessions;
  io_context::strand strand;
  impl(io_context &ioc) : strand(ioc) {}
};

session_mgr::session_mgr(io_context &ioc) { impl_ptr = new impl(ioc); }

session_mgr::~session_mgr() { delete impl_ptr; }

uint64 session_mgr::create_session(tcp::socket &socket,
                                   const std::string &proto_name) {
  auto ret = bind_executor(impl_ptr->strand, [this, &socket, &proto_name] {
    auto new_id = impl_ptr->id_gen.gen();
    auto new_session =
        std::make_shared<tcp_session>(new_id, socket, proto_name);
    impl_ptr->tcp_sessions[new_id] = new_session;
    return new_id;
  })();
  return ret;
}

uint64 session_mgr::create_session(const std::string &proto_name,
                                   const std::string &host,
                                   const std::string &port) {
  auto ret = bind_executor(impl_ptr->strand, [this, &host, &port, &proto_name] {
    tcp::resolver resolver(impl_ptr->strand);
    auto endpoints = resolver.resolve(host, port);
    if (endpoints.size() <= 0) {
      return uint64(0);
    }
    auto new_id = impl_ptr->id_gen.gen();
    auto new_session = std::make_shared<tcp_session>(
        new_id, impl_ptr->strand.context(), proto_name, endpoints);
    impl_ptr->tcp_sessions[new_id] = new_session;
    return new_id;
  })();
  return ret;
}

void session_mgr::remove_session(uint64 id) {
  bind_executor(impl_ptr->strand, [this, &id] {
    impl_ptr->tcp_sessions.erase(id);
    impl_ptr->id_gen.recycle(id);
  })();
}

bool session_mgr::session_valid(uint64 session_id) {
  auto ret = bind_executor(impl_ptr->strand, [this, &session_id] {
    auto iter = impl_ptr->tcp_sessions.find(session_id);
    if (iter == impl_ptr->tcp_sessions.end()) {
      return false;
    }
    return iter->second->valid();
  })();
  return ret;
}
void session_mgr::send_msg(uint64 id, const std::string &msg) {
  bind_executor(impl_ptr->strand, [this, &id, &msg] {
    auto iter = impl_ptr->tcp_sessions.find(id);
    if (iter == impl_ptr->tcp_sessions.end()) {
      return;
    }
    iter->second->send(msg);
  })();
}
