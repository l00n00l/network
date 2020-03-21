#include "tcp_server.h"
#include "session_mgr.h"

struct tcp_server::impl {
  tcp::acceptor acceptor;
  std::string proto_name;
  impl(io_context &ioc, tcp::endpoint &endpoint, const std::string &proto_name)
      : acceptor(ioc, endpoint), proto_name(proto_name) {}
};

tcp_server::tcp_server(io_context &ioc, tcp::endpoint endpoint,
                       const std::string &proto_name) {
  impl_ptr = new impl(ioc, endpoint, proto_name);
  if (!impl_ptr) {
    lserr << "impl_ptr == null" >> __FUNCTION__;
    return;
  }
  _do_accept();
}

tcp_server::~tcp_server() {
  if (impl_ptr)
    delete impl_ptr;
}

void tcp_server::_do_accept() {
  impl_ptr->acceptor.async_accept([this](error_code ec, tcp::socket socket) {
    if (!ec) {
      g_session_mgr->create_session(socket, impl_ptr->proto_name);
    }
    _do_accept();
  });
}
