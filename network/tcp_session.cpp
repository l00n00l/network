#include "tcp_session.h"
#include "session_mgr.h"
#include "tcp_proto.h"

struct tcp_session::impl {
  uint64 id;
  tcp::socket socket;
  exe_strand strand;
  std::unique_ptr<tcp_proto> proto_ptr;
  bool valid;
  impl(uint64 id, tcp::socket &socket, std::string &proto_name)
      : id(id), socket(std::move(socket)), strand(socket.get_executor()),
        valid(false) {
    proto_ptr = create_proto(proto_name);
  }

  impl(uint64 id, io_context &ioc, std::string &proto_name)
      : id(id), socket(ioc), strand(socket.get_executor()) {
    proto_ptr = create_proto(proto_name);
  }
};

tcp_session::tcp_session(uint64 id, tcp::socket &socket,
                         std::string &proto_name) {
  impl_ptr = new impl(id, socket, proto_name);
  _do_read();
  impl_ptr->valid = true;
}

tcp_session::tcp_session(uint64 id, io_context &ioc, std::string &proto_name,
                         std::string &host, std::string &name) {
  impl_ptr = new impl(id, ioc, proto_name);
  tcp::resolver resolver(ioc);
  auto endpoints = resolver.resolve(host, name);
  async_connect(
      impl_ptr->socket, endpoints,
      bind_executor(impl_ptr->strand, [this](error_code ec, tcp::endpoint) {
        if (ec) {
          return;
        }
        _do_read();
        impl_ptr->valid = true;
      }));
}

tcp_session::~tcp_session() { delete impl_ptr; }

bool tcp_session::valid() { return impl_ptr->valid; }

void tcp_session::send(std::string &msg) {
  impl_ptr->proto_ptr->write(msg);
  _do_write();
}

void tcp_session::_do_read() {
  async_read(
      impl_ptr->socket,
      buffer(impl_ptr->proto_ptr->read_buffer_ptr(),
             impl_ptr->proto_ptr->read_buffer_size()),
      bind_executor(impl_ptr->strand, [this](error_code ec, std::size_t size) {
        if (ec) {
          _do_disconnect();
          return;
        }
        impl_ptr->proto_ptr->read_done(size, impl_ptr->id);
        _do_read();
      }));
}

void tcp_session::_do_write() {
  if (!impl_ptr->proto_ptr->has_data_to_write()) {
    return;
  }
  async_write(
      impl_ptr->socket,
      buffer(impl_ptr->proto_ptr->write_buffer_ptr(),
             impl_ptr->proto_ptr->write_buffer_size()),
      bind_executor(impl_ptr->strand, [this](error_code ec, std::size_t size) {
        if (ec) {
          _do_disconnect();
          return;
        }
        impl_ptr->proto_ptr->write_done(size);
        if (impl_ptr->proto_ptr->has_data_to_write()) {
          _do_write();
        }
      }));
}

void tcp_session::_do_disconnect() {
  impl_ptr->valid = false;
  impl_ptr->socket.close();
  g_session_mgr->remove_session(impl_ptr->id);
}
