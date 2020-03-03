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
  virtual void write(const std::string &msg) { write_buffer = msg; }

private:
  char read_buffer[512];
  std::string write_buffer;
};

messge_handler rawtcp_handler = nullptr;
void regist_rawtcp_handler(messge_handler handler) { rawtcp_handler = handler; }
struct tcp_rawtcp : public tcp_proto {
  virtual char *read_buffer_ptr() { return read_buffer; }
  virtual const char *write_buffer_ptr() { return write_buffer.c_str(); }
  virtual std::size_t read_buffer_size() { return read_size; }
  virtual std::size_t write_buffer_size() { return write_buffer.size(); }
  virtual void read_done(std::size_t size, uint64 session_id) {
    if (rawtcp_handler) {
      rawtcp_handler(read_buffer, size, session_id);
    }
  }
  virtual void write_done(std::size_t size) {
    auto cur_size = write_msg_queue.size();
    if (cur_size > 0)
      write_msg_queue.pop_front();
    if (cur_size > 1)
      write_buffer = write_msg_queue.front();
  }
  virtual bool has_data_to_write() { return write_buffer.size() > 0; }
  virtual void write(const std::string &msg) {
    if (write_msg_queue.size() <= 0)
      write_buffer = msg;
    write_msg_queue.push_back(msg);
  }

private:
  enum { read_size = 512 };
  char read_buffer[read_size];
  std::string write_buffer;
  std::list<std::string> write_msg_queue;
};

messge_handler controlhandler = nullptr;
void regist_contol_handler(messge_handler handler) { controlhandler = handler; }

#ifdef _WINDOWS
#define _DLLExport __declspec(dllexport)
#else
#define _DLLExport
#endif // _WINDOWS
extern "C" {
_DLLExport void c_regiest_control_handler(messge_handler handler) {
  controlhandler = handler;
}
}

struct tcp_control : public tcp_proto {
  tcp_control()
      : reading_header(true), body_length(0), read_buffer{0}, read_size(0) {}
  virtual char *read_buffer_ptr() {
    auto ret = reading_header ? read_buffer : read_buffer + head_length;
    return ret + read_size;
  }
  virtual const char *write_buffer_ptr() { return write_buffer.c_str(); }
  virtual std::size_t read_buffer_size() {
    auto ret = reading_header ? head_length : body_length;
    return ret - read_size;
  }
  virtual std::size_t write_buffer_size() { return write_buffer.size(); }
  virtual void read_done(std::size_t size, uint64 session_id) {
    read_size += size;

    if (reading_header && read_size >= head_length) {
      // 读取头部
      _decode_header();
      read_size -= head_length;
      reading_header = !reading_header;
    } else if (read_size >= body_length) {
      // 读取包体
      controlhandler(read_buffer + head_length, body_length, session_id);
      reading_header = !reading_header;
      read_size = 0;
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
  virtual void write(const std::string &msg) {
    std::stringstream ss;
    auto msg_cache = msg;
    auto size = msg.size();
    if (size > max_body_length) {
      size = max_body_length;
      msg_cache = msg.substr(0, max_body_length);
    }
    ss << size;
    auto size_length = ss.str().size();
    ss.str("");
    for (size_t i = 0; i < head_length - size_length; i++) {
      ss << "0";
    }
    ss << size << msg_cache;

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
  enum { head_length = 4, max_body_length = 1024 };
  char read_buffer[head_length + max_body_length + 1];
  bool reading_header;
  std::size_t read_size;
  std::size_t body_length;
  std::list<std::string> write_buffer_list;
  std::string write_buffer;
};

#define make_proto(name) std ::make_unique<tcp_##name>()

std::unique_ptr<tcp_proto> create_proto(const std::string &proto_name) {
  if (proto_name == "echo") {
    return make_proto(echo);
  } else if (proto_name == "rawtcp") {
    return make_proto(rawtcp);
  } else if (proto_name == "control") {
    return make_proto(control);
  }
  return nullptr;
}