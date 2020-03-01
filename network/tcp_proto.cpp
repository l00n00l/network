#include "tcp_proto.h"
#include <unordered_map>

struct tcp_echo : public tcp_proto {
  virtual char *read_buffer_ptr() { return read_buffer; }
  virtual const char *write_buffer_ptr() { return write_buffer.c_str(); }
  virtual std::size_t read_buffer_size() { return 10; }
  virtual std::size_t write_buffer_size() { return write_buffer.size(); }
  virtual void read_done(std::size_t size, uint64 session_id) {
    std::cout.write(read_buffer, size);
    std::cout << std::endl;
  }
  virtual void write_done(std::size_t size) { write_buffer = ""; }
  virtual bool has_data_to_write() { return write_buffer.size() > 0; }
  virtual void write(std::string &msg) { write_buffer = msg; }

private:
  char read_buffer[512];
  std::string write_buffer;
};

struct tcp_clienttest : public tcp_proto {
  virtual char *read_buffer_ptr() { return read_buffer; }
  virtual const char *write_buffer_ptr() { return write_buffer.c_str(); }
  virtual std::size_t read_buffer_size() { return 512; }
  virtual std::size_t write_buffer_size() { return write_buffer.size(); }
  virtual void read_done(std::size_t size, uint64 session_id) {}
  virtual void write_done(std::size_t size) {}
  virtual bool has_data_to_write() { return write_buffer.size() > 0; }
  virtual void write(std::string &msg) { write_buffer = msg; }

private:
  char read_buffer[512];
  std::string write_buffer;
};

control_handler controlhandler = nullptr;
void regist_contol_handler(control_handler handler) {
  controlhandler = handler;
}

extern "C" {
__declspec(dllexport) void c_regiest_control_handler(control_handler handler) {
  controlhandler = handler;
}
}

struct tcp_control : public tcp_proto {
  tcp_control() : reading_header(true), body_length(0), read_buffer{0} {}
  virtual char *read_buffer_ptr() {
    return reading_header ? read_buffer : read_buffer + head_length;
  }
  virtual const char *write_buffer_ptr() { return write_buffer.c_str(); }
  virtual std::size_t read_buffer_size() {
    return reading_header ? head_length : body_length;
  }
  virtual std::size_t write_buffer_size() { return write_buffer.size(); }
  virtual void read_done(std::size_t size, uint64 session_id) {
    if (reading_header) {
      _decode_header();
    }
    reading_header = !reading_header;
    if (reading_header) {
      controlhandler(read_buffer + head_length, size, session_id);
    }
  }
  virtual void write_done(std::size_t size) {
    if (write_buffer_list.size() > 0) {
      write_buffer_list.pop_front();
      if (write_buffer_list.size() > 0) {
        write_buffer = write_buffer_list.front();
      }
    }
  }
  virtual bool has_data_to_write() { return write_buffer_list.size() > 0; }
  virtual void write(std::string &msg) {
    std::stringstream ss;
    auto size = msg.size();
    if (size > max_body_length) {
      size = max_body_length;
      msg = msg.substr(0, max_body_length);
    }
    ss << size;
    auto size_length = ss.str().size();
    ss.str("");
    for (size_t i = 0; i < head_length - size_length; i++) {
      ss << "0";
    }
    ss << size << msg;

    if (write_buffer.size() <= 0) {
      write_buffer = ss.str();
    }

    write_buffer_list.push_back(ss.str());
  }

private:
  void _decode_header() {
    char header[head_length + 1]{""};
    std::strncat(header, read_buffer, head_length);
    body_length = std::atoi(header);
    if (body_length > max_body_length) {
      body_length = max_body_length;
    }
  }

private:
  enum { head_length = 4, max_body_length = 512 };
  char read_buffer[head_length + max_body_length + 1];
  bool reading_header;
  std::size_t body_length;
  std::list<std::string> write_buffer_list;
  std::string write_buffer;
};

#define make_proto(name) std ::make_unique<tcp_##name>()

std::unique_ptr<tcp_proto> create_proto(std::string &proto_name) {
  if (proto_name == "echo") {
    return make_proto(echo);
  } else if (proto_name == "clienttest") {
    return make_proto(clienttest);
  } else if (proto_name == "control") {
    return make_proto(control);
  }
}