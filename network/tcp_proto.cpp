#include "tcp_proto.h"
#include "../utils/dicts.h"
#include "../utils/json.h"
#include "../utils/regex_string.h"
#include "tcp_proto.h"
#include <unordered_map>
#include <vector>
struct read_item {
  read_type rtype;
  union {
    std::size_t size;
    char *var_name;
    char *until_regex_str;
  } arg;
  regex compile_regex;
  regex next_condition_regex;
  std::list<std::string> var_names;
  read_item() = default;
  read_item(read_item &o) = delete;
  read_item(read_item &&o) noexcept {
    rtype = o.rtype;
    switch (rtype) {
    case read_until:
      arg.until_regex_str = o.arg.until_regex_str;
      o.arg.until_regex_str = nullptr;
      break;
    case read_size:
      arg.size = o.arg.size;
      break;
    case read_var_size:
      arg.var_name = o.arg.var_name;
      o.arg.var_name = nullptr;
      break;
    }
    compile_regex = std::move(o.compile_regex);
    next_condition_regex = std::move(o.next_condition_regex);
    var_names = std::move(o.var_names);
  }
  ~read_item() {
    switch (rtype) {
    case read_until:
      if (arg.until_regex_str)
        delete[] arg.until_regex_str;
      break;
    case read_var_size:
      if (arg.var_name)
        delete[] arg.var_name;
      break;
    }
  }
};

typedef std::vector<read_item> read_item_list;
typedef read_item_list read_lists[2];
typedef std::unordered_map<std::string, read_lists> protos_map;
protos_map proto_map;

dicts g_net_dicts;
message_handler_type g_msg_handler = nullptr;
std::atomic_flag handler_flag;
void set_message_handler(message_handler_type handler) {
  g_msg_handler = handler;
}

void msg_handler(uint64 session_id, uint64 data_id, const char *proto_name) {
  atomic_flag_acquire(handler_flag);
  if (g_msg_handler)
    g_msg_handler(session_id, data_id, proto_name);
  atomic_flag_release(handler_flag);
}

std::unique_ptr<tcp_proto> create_proto_server(const std::string &proto_name) {
  auto iter = proto_map.find(proto_name);
  if (iter == proto_map.end())
    return nullptr;
  return std::make_unique<tcp_proto>(proto_name, side_server);
}

std::unique_ptr<tcp_proto> create_proto_client(const std::string &proto_name) {
  auto iter = proto_map.find(proto_name);
  if (iter == proto_map.end())
    return nullptr;
  return std::make_unique<tcp_proto>(proto_name, side_client);
}

inline bool load_read_items(Value &v, const std::string &proto_name,
                            const std::string &sidename, read_item_list &rl) {
  if (v.Size() <= 0) {
    lserr << proto_name << "[" << sidename >> u8"]不能为空";
    return false;
  }

  read_item temp_item;

  for (auto i = 0; i < (int32)v.Size(); ++i) {
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
      auto size = v[i]["read_until"].GetStringLength() + (std::size_t)1;
      temp_item.arg.until_regex_str = new char[size];
      strcpy(temp_item.arg.until_regex_str, v[i]["read_until"].GetString(),
             size);
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
        auto size = v[i]["read_size"].GetStringLength() + (std::size_t)1;
        temp_item.arg.var_name = new char[size];
        strcpy(temp_item.arg.var_name, v[i]["read_size"].GetString(), size);
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

    regstring s;
    if (!s.parse_regex(v[i]["complie_regex"].GetString(),
                       v[i]["complie_regex"].GetStringLength())) {
      lserr << proto_name << "[" << sidename << "][" << i >>
          u8"][complie_regex]表达式错误，请检查！";
      return false;
    };

    temp_item.compile_regex.set_expression(
        v[i]["complie_regex"].GetString(),
        v[i]["complie_regex"].GetStringLength());

    temp_item.var_names = s.names();

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

  return true;
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
  std::string name;
  enum { read_some_buffer_size = 1024 };
  char buffer[read_some_buffer_size + 1];
  std::size_t buffer_offset;
  uint64 net_vars_id;
  std::string dbuffer;
  std::size_t dbuffer_size;
  asio_stream_buf sbuffer;
  read_item_list &proto_data;
  int32 cur_index;
  std::string write_buffer;
  std::list<std::string> write_msgs;
  regex re;
  impl(const std::string &proto_name, side_index side)
      : name(proto_name), net_vars_id(g_net_dicts.gen()), buffer{0},
        buffer_offset(0), dbuffer_size(0),
        proto_data(proto_map[proto_name][side]), cur_index(-1) {
    next();
  }
  ~impl() {
    if (net_vars_id != 0) {
      g_net_dicts.remove(net_vars_id);
    }
  }
  inline enum read_type cur_read_type() { return proto_data[cur_index].rtype; }
  inline read_item &read_item() { return proto_data[cur_index]; }
  inline void next() {
    cur_index = (cur_index + (int32)1) % proto_data.size();
    buffer_offset = 0;
    switch (cur_read_type()) {
    case read_some: {
      auto sb_size = sbuffer.size();
      if (sb_size > 0) {
        if (sb_size > read_some_buffer_size)
          sb_size = read_some_buffer_size;
        auto s = sbuffer_string(sbuffer, sb_size);
        strcpy(buffer, s.c_str(), sb_size);
        buffer_offset = sb_size;
      }
    } break;
    case read_size: {
      dbuffer_size = read_item().arg.size;
      auto sb_size = sbuffer.size();
      if (sb_size > 0) {
        if (sb_size > dbuffer_size)
          sb_size = dbuffer_size;
        dbuffer = sbuffer_string(sbuffer, sb_size);
      }
    } break;
    case read_var_size: {
      auto sb_size = sbuffer.size();
      if (sb_size > 0) {
        auto var_value =
            g_net_dicts.get_string(net_vars_id, read_item().arg.var_name);

        if (!var_value.empty()) {
          dbuffer_size = to_uint64(var_value);
        } else {
          lserr << u8"协议错误!找不到变量" << read_item().arg.var_name >>
              __FUNCTION__;
        }

        if (sb_size > dbuffer_size)
          sb_size = dbuffer_size;
        dbuffer = sbuffer_string(sbuffer, sb_size);
      }
    } break;
    case read_until:
      re.set_expression(read_item().arg.until_regex_str);
      break;
    }
  }
};

tcp_proto::tcp_proto(const std::string &proto_name, side_index side) {
  impl_ptr = new impl(proto_name, side);
}

tcp_proto::~tcp_proto() {
  if (impl_ptr)
    delete impl_ptr;
}

read_type tcp_proto::cur_read_type() { return impl_ptr->cur_read_type(); }

char *tcp_proto::read_buffer_ptr() {
  return impl_ptr->buffer + impl_ptr->buffer_offset;
}

std::string &tcp_proto::dbuffer() { return impl_ptr->dbuffer; }

asio_stream_buf &tcp_proto::sbuffer() { return impl_ptr->sbuffer; }

const char *tcp_proto::write_buffer_ptr() {
  return impl_ptr->write_buffer.c_str();
}

std::size_t tcp_proto::read_buffer_size() {
  switch (cur_read_type()) {
  case read_some:
    return impl::read_some_buffer_size - impl_ptr->buffer_offset;
  case read_size:
  case read_var_size:
    return impl_ptr->dbuffer_size;
  }
  return 0;
}

regex &tcp_proto::until_regex() { return impl_ptr->re; }

std::size_t tcp_proto::write_buffer_size() {
  return impl_ptr->write_buffer.size();
}

inline bool compile_read_data(const std::string &data, regex &r, uint64 var_id,
                              std::list<std::string> &names) {
  try {
    match_results_str mret;
    if (!regex_search(data, mret, r)) {
      lserr << u8"协议解析错误，请检测协议解析表达式!" >> __FUNCTION__;
      return false;
    }

    for (auto &name : names) {
      g_net_dicts.set_string(var_id, name, mret[name]);
    }
  } catch (std::exception &e) {
    lserr << __FUNCTION__ >> std::string(e.what());
    return false;
  } catch (...) {
    lserr << u8"数据解析错误!数据=" << data << u8"解析表达式为=" << r.str()
          << u8"。" >>
        __FUNCTION__;
    return false;
  }

  return true;
}

void tcp_proto::read_done(std::size_t size, uint64 session_id) {
  std::string data = "";
  switch (impl_ptr->cur_read_type()) {
  case read_some:
    data = std::string(impl_ptr->buffer, size);
    break;
  case read_until:
    data = sbuffer_string(impl_ptr->sbuffer, size);
    break;
  case read_size:
  case read_var_size:
    data = impl_ptr->dbuffer;
    impl_ptr->dbuffer.clear();
    break;
  }

  auto succeed =
      compile_read_data(data, impl_ptr->read_item().compile_regex,
                        impl_ptr->net_vars_id, impl_ptr->read_item().var_names);

  if (succeed) {
    // 读取完整个协议后才执行handler
    if (impl_ptr->cur_index == impl_ptr->proto_data.size() - 1) {
      msg_handler(session_id, impl_ptr->net_vars_id, impl_ptr->name.c_str());
    }
    // 读取下一项
    impl_ptr->next();
  }
}

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
