#include "tcp_session.h"
#include "session_mgr.h"
#include "tcp_proto.h"

struct tcp_session::impl {
  uint64 id;
  tcp::socket socket;
  exe_strand strand;
  std::unique_ptr<tcp_proto> proto_ptr;
  bool valid;
  bool writing;
  bool reconnecting;
  steady_timer reconnect_timer;
  tcp_resolve_result endpoints;
  impl(uint64 id, tcp::socket &socket, const std::string &proto_name)
      : id(id), socket(std::move(socket)), strand(socket.get_executor()),
        valid(false), writing(false), reconnect_timer(socket.get_executor()),
        reconnecting(false) {
    proto_ptr = create_proto(proto_name, "server");
  }

  impl(uint64 id, io_context &ioc, const std::string &proto_name)
      : id(id), socket(ioc), strand(socket.get_executor()), valid(false),
        writing(false), reconnect_timer(socket.get_executor()),
        reconnecting(false) {
    proto_ptr = create_proto(proto_name, "client");
  }
};

tcp_session::tcp_session(uint64 id, tcp::socket &socket,
                         const std::string &proto_name) {
  impl_ptr = new impl(id, socket, proto_name);
  impl_ptr->valid = true;
  _do_read();
}

tcp_session::tcp_session(uint64 id, io_context &ioc,
                         const std::string &proto_name,
                         tcp_resolve_result endpoints) {
  impl_ptr = new impl(id, ioc, proto_name);
  impl_ptr->endpoints = endpoints;
  if (impl_ptr->endpoints.size() > 0) {
    _do_connect();
  }
}

tcp_session::~tcp_session() { delete impl_ptr; }

bool tcp_session::valid() { return impl_ptr->valid; }

void tcp_session::send(const std::string &msg) {
  bind_executor(impl_ptr->strand, [this, &msg] {
    impl_ptr->proto_ptr->write(msg);
    if (impl_ptr->writing == false) {
      _do_write();
    }
  })();
}

void tcp_session::_do_read() {
  if (!(impl_ptr->valid)) {
    if (impl_ptr->endpoints.size() > 0)
      _set_reconnect_timer();
    return;
  }

  switch (impl_ptr->proto_ptr->read_type()) {
  case 0: {
    _do_read_some();
    break;
  }
  case 1: {
    _do_read_by_size();
    break;
  }
  case 2: {
    _do_read_until();
    break;
  }
  default:
    _do_disconnect();
    break;
  }
}

void tcp_session::_do_read_some() {
  impl_ptr->socket.async_read_some(
      buffer(impl_ptr->proto_ptr->read_buffer_ptr(),
             impl_ptr->proto_ptr->read_buffer_size()),
      bind_executor(impl_ptr->strand, [this](error_code ec, std::size_t size) {
        if (ec) {
          if (impl_ptr->endpoints.size() <= 0) {
            _do_disconnect();
          } else {
            _set_reconnect_timer();
          }
          return;
        }
        impl_ptr->proto_ptr->read_done(size, impl_ptr->id);
        _do_read();
      }));
}

void tcp_session::_do_read_by_size() {
  async_read(
      impl_ptr->socket,
      dynamic_buffer(impl_ptr->proto_ptr->dbuffer(),
                     impl_ptr->proto_ptr->read_buffer_size()),
      bind_executor(impl_ptr->strand, [this](error_code ec, std::size_t size) {
        if (ec) {
          if (impl_ptr->endpoints.size() <= 0) {
            _do_disconnect();
          } else {
            _set_reconnect_timer();
          }
          return;
        }
        impl_ptr->proto_ptr->read_done(size, impl_ptr->id);
        _do_read();
      }));
}

void tcp_session::_do_read_until() {
  async_read_until(
      impl_ptr->socket, impl_ptr->proto_ptr->sbuffer(),
      impl_ptr->proto_ptr->until_regex(),
      bind_executor(impl_ptr->strand, [this](error_code ec, std::size_t size) {
        if (ec) {
          if (impl_ptr->endpoints.size() <= 0) {
            _do_disconnect();
          } else {
            _set_reconnect_timer();
          }
          return;
        }
        impl_ptr->proto_ptr->read_done(size, impl_ptr->id);
        _do_read();
      }));
}

void tcp_session::_do_write() {
  if (!(impl_ptr->valid) && impl_ptr->endpoints.size() > 0) {
    _set_reconnect_timer();
    return;
  }
  if (!(impl_ptr->proto_ptr->has_data_to_write())) {
    return;
  }
  impl_ptr->writing = true;
  async_write(
      impl_ptr->socket,
      buffer(impl_ptr->proto_ptr->write_buffer_ptr(),
             impl_ptr->proto_ptr->write_buffer_size()),
      bind_executor(impl_ptr->strand, [this](error_code ec, std::size_t size) {
        impl_ptr->writing = false;
        if (ec) {
          if (impl_ptr->endpoints.size() <= 0) {
            _do_disconnect();
          } else {
            _set_reconnect_timer();
          }
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

void tcp_session::_do_connect() {
  async_connect(
      impl_ptr->socket, impl_ptr->endpoints,
      bind_executor(impl_ptr->strand, [this](error_code ec, tcp::endpoint) {
        if (ec) {
          _set_reconnect_timer();
          return;
        }
        impl_ptr->valid = true;
        _do_read_some();
      }));
}

void tcp_session::_set_reconnect_timer() {
  if (impl_ptr->reconnecting) {
    return;
  }
  impl_ptr->reconnecting = true;
  impl_ptr->valid = false;
  impl_ptr->socket.close();
  impl_ptr->socket = tcp::socket(impl_ptr->socket.get_executor());
  impl_ptr->reconnect_timer.expires_after(std::chrono::seconds(3));
  impl_ptr->reconnect_timer.async_wait([this](error_code ec2) {
    impl_ptr->reconnecting = false;
    _do_connect();
  });
}
