#include "server_mgr.h"
#include "../utils/id_generator.h"
#include "tcp_server.h"
#include <unordered_map>

std::shared_ptr<server_mgr> g_server_mgr;

typedef std::unordered_map<uint64, std::shared_ptr<tcp_server>> tcp_server_map;

struct server_mgr::impl {
  io_context &ioc;
  tcp_server_map servers;
  id_generator id_gen;
  io_context::strand strand;
  impl(io_context &ioc) : ioc(ioc), strand(ioc) {}
};

server_mgr::server_mgr(io_context &ioc) { impl_ptr = new impl(ioc); }

server_mgr::~server_mgr() { delete impl_ptr; }

void server_mgr::create_server(tcp::endpoint endpoint,
                               std::string &proto_name) {
  bind_executor(impl_ptr->strand, [this, &endpoint, &proto_name] {
    auto new_id = impl_ptr->id_gen.gen();
    auto new_server =
        std::make_shared<tcp_server>(impl_ptr->ioc, endpoint, proto_name);
    impl_ptr->servers[new_id] = new_server;
  })();
}

void server_mgr::destroy_server(uint64 server_id) {
  bind_executor(impl_ptr->strand,
                [this, &server_id] { impl_ptr->servers.erase(server_id); })();
}
