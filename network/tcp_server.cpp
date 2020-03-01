#include "tcp_server.h"
#include "session_mgr.h"

struct tcp_server::impl {
  tcp::acceptor acceptor;
  std::string proto_name;
  impl(io_context &ioc, tcp::endpoint &endpoint, std::string &proto_name)
      : acceptor(ioc, endpoint), proto_name(proto_name) {}
};

tcp_server::tcp_server(io_context &ioc, tcp::endpoint endpoint,
                       std::string &proto_name) {
  impl_ptr = new impl(ioc, endpoint, proto_name);
  _do_accept();
}

tcp_server::~tcp_server() { delete impl_ptr; }

void tcp_server::_do_accept() {
  impl_ptr->acceptor.async_accept([this](error_code ec, tcp::socket socket) {
    if (!ec) {
      g_session_mgr->create_session(socket, impl_ptr->proto_name);
    }
    _do_accept();
  });
}
