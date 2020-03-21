#include "tcp_proto.h"
#include "../utils/dicts.h"
#include "../utils/json.h"
#include "tcp_proto.h"
#include <unordered_map>

struct read_item {
  read_type rtype;
  union {
    std::size_t size;
    char *var_name;
  } arg;
  regex compile_regex;
  regex next_condition_regex;
  read_item() = default;
  read_item(read_item &o) = delete;
  read_item(read_item &&o) noexcept {
    rtype = o.rtype;
    if (rtype == read_var_size) {
      arg.var_name = o.arg.var_name;
      o.arg.var_name = nullptr;
    } else if (rtype == read_size) {
      arg.size = o.arg.size;
    }
    compile_regex = std::move(o.compile_regex);
    next_condition_regex = std::move(o.next_condition_regex);
  }
  ~read_item() {
    if (rtype == read_var_size && arg.var_name != nullptr) {
      free(arg.var_name);
    }
  }
};

typedef std::list<read_item> read_item_list;
typedef std::list<read_item> read_lists[2];
typedef std::unordered_map<std::string, read_lists> protos_map;
protos_map proto_map;

dicts g_net_dicts;
message_handler_type g_msg_handler = nullptr;
std::atomic_flag handler_flag;
void msg_handler(uint64 session_id, uint64 data_id, const char *proto_name) {
  atomic_flag_acquire(handler_flag);
  if (g_msg_handler)
    g_msg_handler(session_id, data_id, proto_name);
  atomic_flag_release(handler_flag);
}

std::unique_ptr<tcp_proto> create_proto_server(const std::string &proto_name) {
  return nullptr;
}

std::unique_ptr<tcp_proto> create_proto_client(const std::string &proto_name) {
  return nullptr;
}

inline bool load_read_items(Value &v, const std::string &proto_name,
                            const std::string &sidename, read_item_list &rl) {
  if (v.Size() <= 0) {
    lserr << proto_name << "[" << sidename >> u8"]不能为空";
    return false;
  }

  read_item temp_item;

  for (auto i = 0; i < v.Size(); ++i) {
    // 首先读取项得是一个对象
    if (!v[i].IsObject()) {
      lserr << proto_name << "[" << sidename << "][" << i >>
          u8"]应该是对象{}！";
      return false;
    }

    // read_until读取方式检查 必须是正则字符串
    if (v[i].HasMember("read_until")) {
      if (!v[i]["read_until"].IsString()) {
        lserr << proto_name << "[" << sidename << "][" << i >>
            u8"][read_until]不是字符串";
        return false;
      }

      temp_item.rtype = read_until;
    }

    // read_size读取方式检查
    // 必须是变量引用（$varname）varname必须在上面的compile_regex出现过或者数字
    else if (v[i].HasMember("read_size")) {
      if (v[i]["read_size"].IsString()) {
        if (regex_match(value_to_string(v[i]["read_size"]), regex("\\&.*"))) {
          lserr << proto_name << "[" << sidename << "][" << i >>
              u8"][read_size]格式错误！应该是$name";
          return false;
        }
        temp_item.rtype = read_var_size;
        temp_item.arg.var_name = (char *)malloc(
            v[i]["read_size"].GetStringLength() + (std::size_t)1);
        strcpy(temp_item.arg.var_name, v[i]["read_size"].GetString());

      } else if (!v[i]["read_size"].IsUint()) {
        lserr << proto_name << "[" << sidename << "][" << i >>
            u8"][read_size]应该是字符串或者正整数！";
        return false;
        temp_item.rtype = read_size;
        temp_item.arg.size = v[i]["read_size"].GetUint();
      }
    } else {
      temp_item.rtype = read_some;
    }

    // 数据解析正则表达式（根据正则表达式将数据解析为可用的变量）
    if (!v[i].HasMember("complie_regex")) {
      lserr << proto_name << "[" << sidename << "][" << i >>
          u8"][complie_regex]不能存在！";
      return false;
    } else if (!v[i]["complie_regex"].IsString()) {
      lserr << proto_name << "[" << sidename << "][" << i >>
          u8"][complie_regex]应该是字符串！";
      return false;
    }

    temp_item.compile_regex.set_expression(
        v[i]["complie_regex"].GetString(),
        v[i]["complie_regex"].GetStringLength());

    // 执行完毕正则表达式(用于判定此项的数据读取是否完毕，如果判定失败则继续执行此项)
    if (!v[i].HasMember("next_condition")) {
      lserr << proto_name << "[" << sidename << "][" << i >>
          u8"][next_condition]不能存在！";
      return false;
    } else if (!v[i]["next_condition"].IsString()) {
      lserr << proto_name << "[" << sidename << "][" << i >>
          u8"][next_condition]应该是字符串！";
      return false;
    }

    temp_item.next_condition_regex.set_expression(
        v[i]["next_condition"].GetString(),
        v[i]["next_condition"].GetStringLength());

    rl.push_back(std::move(temp_item));
  }
}

inline bool proto_check(Document &d, const std::string proto_name,
                        read_lists &rl) {
  if (!d.HasMember("server") || !d.HasMember("client")) {
    lserr << proto_name >> u8"没有 server 或者 client 字段";
    return false;
  }

  if (!d["server"].IsArray()) {
    lserr << proto_name >> u8" server 不是数组";
    return false;
  }

  if (!d["client"].IsArray()) {
    lserr << proto_name >> u8" client 不是数组";
    return false;
  }

  if (!load_read_items(d["server"], proto_name, "server", rl[side_server])) {
    return false;
  }

  if (!load_read_items(d["client"], proto_name, "client", rl[side_client])) {
    return false;
  }

  return true;
}
bool load_protos(const std::string &path) {
  try {
    fsys::path p(path);
    if (!fsys::exists(p) || !fsys::is_directory(p))
      return false;
    fsys::directory_iterator end;
    for (fsys::directory_iterator iter(p); iter != end; ++iter) {
      auto d = json_load_fs(iter->path().string().c_str());
      auto filname = iter->path().filename().string();
      filname = filname.substr(0, filname.size() - 5);
      if (!proto_check(d, iter->path().string(), proto_map[filname]))
        return false;
    }
  } catch (const std::exception &e) {
    lserr << u8"加载协议错误。异常：" >> std::string(e.what());
    return false;
  }
  return true;
}

struct tcp_proto::impl {
  enum { read_some_buffer_size = 1024 };
  char buffer[read_some_buffer_size];
  uint64 net_vars_id;
  std::string dbuffer;
  std::size_t dbuffer_size;
  asio_stream_buf sbuffer;
  read_item_list &proto_data;
  int32 cur_index;
  std::string write_buffer;
  std::list<std::string> write_msgs;
  regex re;
  enum read_type cur_read_type;
  enum read_type pre_read_type;
  impl(const std::string &proto_name, side_index side)
      : net_vars_id(g_net_dicts.gen()), buffer{0}, dbuffer_size(0),
        proto_data(proto_map[proto_name][side]), cur_index(-1),
        cur_read_type(none), pre_read_type(none) {}
  ~impl() {
    if (net_vars_id != 0) {
      g_net_dicts.remove(net_vars_id);
    }
  }
  void next() {}
};

tcp_proto::tcp_proto(const std::string &proto_name, side_index side) {
  impl_ptr = new impl(proto_name, side);
  if (!impl_ptr) {
    lserr << "impl_ptr == null" >> __FUNCTION__;
    return;
  }
}

tcp_proto::~tcp_proto() {
  if (impl_ptr)
    delete impl_ptr;
}

read_type tcp_proto::cur_read_type() { return impl_ptr->cur_read_type; }

char *tcp_proto::read_buffer_ptr() { return impl_ptr->buffer; }

std::string &tcp_proto::dbuffer() { return impl_ptr->dbuffer; }

asio_stream_buf &tcp_proto::sbuffer() { return impl_ptr->sbuffer; }

const char *tcp_proto::write_buffer_ptr() {
  return impl_ptr->write_buffer.c_str();
}

std::size_t tcp_proto::read_buffer_size() {
  return impl::read_some_buffer_size;
}

regex &tcp_proto::until_regex() { return impl_ptr->re; }

std::size_t tcp_proto::write_buffer_size() {
  return impl_ptr->write_buffer.size();
}

void tcp_proto::read_done(std::size_t size, uint64 session_id) {}

void tcp_proto::write_done(std::size_t size) {
  auto cur_size = impl_ptr->write_msgs.size();
  if (cur_size > 0)
    impl_ptr->write_msgs.pop_front();
  if (cur_size > 1)
    impl_ptr->write_buffer = impl_ptr->write_msgs.front();
}

bool tcp_proto::has_data_to_write() { return impl_ptr->write_msgs.size() > 0; }

void tcp_proto::write(const std::string &msg) {
  if (impl_ptr->write_msgs.size() <= 0)
    impl_ptr->write_buffer = msg;
  impl_ptr->write_msgs.push_back(msg);
}
