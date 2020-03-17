#include "tcp_proto.h"
#include "../utils/dicts.h"
#include "../utils/json.h"
#include <unordered_map>

dicts g_net_dicts;
message_handler_type g_msg_handler = nullptr;
std::atomic_flag handler_flag;
void msg_handler(uint64 session_id, uint64 data_id, const char *proto_name) {
  atomic_flag_acquire(handler_flag);
  if (g_msg_handler)
    g_msg_handler(session_id, data_id, proto_name);
  atomic_flag_release(handler_flag);
}

std::unique_ptr<tcp_proto> create_proto(const std::string &proto_name,
                                        const std::string &server_or_client) {
  return nullptr;
}

std::unordered_map<std::string, Document> proto_map;
auto read_names = {"read_regex", "read_size", "read_some"};
bool proto_check(Document &d, const std::string proto_name) {
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

  for (auto iter = d.MemberBegin(); iter != d.MemberEnd(); ++iter) {
    if (iter->value.Size() <= 0) {
      lserr << proto_name << "[" << value_to_string(iter->name) >>
          u8"]不能为空";
      return false;
    }

    for (auto i = 0; i < iter->value.Size(); ++i) {
      // 首先读取项得是一个对象
      if (!iter->value[i].IsObject()) {
        lserr << proto_name << "[" << value_to_string(iter->name) << "]["
              << i >>
            u8"]不是对象";
        return false;
      }

      // 【读取方式】(read_regex | read_size | read_some) 必须时String类型
      bool flag_has_creact_fieldname = false;
      for (auto &read_name : read_names) {
        if (iter->value[i].HasMember(read_name)) {
          flag_has_creact_fieldname = true;
          if (!iter->value[i][read_name].IsString()) {
            if (read_name == "read_size" &&
                !iter->value[i][read_name].IsInt()) {
              lserr << proto_name << "[" << value_to_string(iter->name) << "]["
                    << i << u8"][" << read_name >>
                  u8"]不是整数也不是字符串";
            } else {
              lserr << proto_name << "[" << value_to_string(iter->name) << "]["
                    << i << u8"][" << read_name >>
                  u8"]不是字符串";
            }

            return false;
          } else if (read_name == "read_size") {
            match_results_str mrets;
            if (!regex_search(value_to_string(iter->value[i][read_name]), mrets,
                              regex("(?=\\$).*"))) {
            }
          }

          Value read_type(kStringType);
          read_type.SetString(read_name, d.GetAllocator());
          iter->value[i].AddMember("read_type", read_type, d.GetAllocator());
          break;
        }
      }

      if (!flag_has_creact_fieldname) {
        lserr << proto_name << "[" << value_to_string(iter->name) << "]["
              << i >>
            u8"]没有(read_regex | read_size | read_some)";
        return false;
      }

      // 【要生成的变量（vars）】【String】对应正则表达式的匹配结果索引
      if (!iter->value[i].HasMember("vars")) {
        lserr << proto_name << "[" << value_to_string(iter->name) << "]["
              << i >>
            u8"]没有vars";
        return false;
      }
      // vars应该时一个数组
      if (!iter->value[i]["vars"].IsArray()) {
        lserr << proto_name << "[" << value_to_string(iter->name) << "]["
              << i >>
            u8"][vars]不是数组";
        return false;
      }

      // 数组内包含的应该是字符串
      for (auto &item : iter->value[i]["vars"].GetArray()) {
        if (!item.IsString()) {
          lserr << proto_name << "[" << value_to_string(iter->name) << "][" << i
                << u8"][vars]中不全是字符串:" >>
              value_to_string(item);
          return false;
        }
      }

      // 【通向下一组的条件（next_condition）】应该是对象
      if (!iter->value[i].HasMember("next_condition")) {
        lserr << proto_name << "[" << value_to_string(iter->name) << "]["
              << i >>
            u8"]没有next_condition";
        return false;
      }

      if (!iter->value[i]["next_condition"].IsObject()) {
        lserr << proto_name << "[" << value_to_string(iter->name) << "]["
              << i >>
            u8"][next_condition]不是对象";
        return false;
      }
    }
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
      if (!proto_check(d, iter->path().string()))
        return false;
      auto filname = iter->path().filename().string();
      filname = filname.substr(0, filname.size() - 5);
      proto_map[filname].Swap(d);
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
  Value &proto_data;
  int32 cur_index;
  std::string write_buffer;
  std::list<std::string> write_msgs;
  regex re;
  uint8 cur_read_type;
  uint8 pre_read_type;
  impl(const std::string &proto_name, const std::string &side)
      : net_vars_id(g_net_dicts.gen()), buffer{0}, dbuffer_size(0),
        proto_data(proto_map[proto_name][side.c_str()]), cur_index(-1),
        cur_read_type(0XFF), pre_read_type(0XFF) {}
  ~impl() {
    if (net_vars_id != 0) {
      g_net_dicts.remove(net_vars_id);
    }
  }
  void next() {
    // 缓存上一次的读取方式
    pre_read_type = cur_read_type;

    // 增长索引
    cur_index = (cur_index + 1) % proto_data.GetArray().Size();

    // 获取读取方式
    auto &node = proto_data[cur_index];
    auto read_type = value_to_string(node["read_type"]);
    if (read_type == "read_some") {
      cur_read_type = 0;
    } else if (read_type == "read_size") {
      cur_read_type = 1;
      if (node["read_size"].IsInt()) {
        dbuffer_size = node["read_size"].GetInt();
      } else {
        match_results_str mrets;
        if (!regex_search(value_to_string(node["read_size"]), mrets,
                          regex("(?=\\$).*"))) {
          dbuffer_size = 0;
        }
      }

    } else if (read_type == "read_regex") {
      cur_read_type = 2;
      re.set_expression(
          value_to_string(node["read_regex"])); // 确定读取终止正则
    }
  }
};

tcp_proto::tcp_proto(const std::string &proto_name, const std::string &side) {
  impl_ptr = new impl(proto_name, side);
}

tcp_proto::~tcp_proto() {
  if (impl_ptr)
    delete impl_ptr;
}

uint8 tcp_proto::read_type() { return impl_ptr->cur_read_type; }

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

void tcp_proto::read_done(std::size_t size, uint64 session_id) {
  //// 读取相应的数据
  // std::string data;
  // switch (impl_ptr->cur_read_type) {
  // case 0: {
  //  data = std::string(impl_ptr->buffer, size);
  //} break;
  // case 1: {
  //  data = impl_ptr->dbuffer;
  //  impl_ptr->dbuffer.clear();
  //} break;
  // case 2: {
  //  data = std::string(buffers_begin(impl_ptr->sbuffer.data()),
  //                     buffers_begin(impl_ptr->sbuffer.data()) + size);
  //  impl_ptr->sbuffer.consume(size);
  //} break;
  //}

  //// 根据数据确定协议变量
  // auto iter = impl_ptr->proto_data.FindMember("vars");
  // if (iter != impl_ptr->proto_data.MemberEnd()) {
  //  std::unordered_map<uint32, std::string> key_cache;
  //  std::unordered_map<uint32, std::string> value_cache;
  //  std::string data_type;
  //  if (!iter->value.IsObject()) {
  //    lserr << impl_ptr->protoname << "_" << impl_ptr->side << "_"
  //          << impl_ptr->cur_index >>
  //        u8" var 错误";
  //    return;
  //  }
  //  for (auto itr = iter->value.MemberBegin(); itr != iter->value.MemberEnd();
  //       ++itr) {

  //    auto func_type_iter = itr->value.FindMember("func_type");
  //    if (func_type_iter == itr->value.MemberEnd() &&
  //        !func_type_iter->value.IsString()) {
  //      break;
  //    }

  //    auto func_type = std::string(func_type_iter->value.GetString(),
  //                                 func_type_iter->value.GetStringLength());
  //    if (func_type == "regex") {
  //      auto value_iter = itr->value.FindMember("func_type");
  //      if (value_iter == itr->value.MemberEnd() &&
  //          !value_iter->value.IsString()) {
  //        break;
  //      }

  //      regex r_data(std::string(value_iter->value.GetString(),
  //                               value_iter->value.GetStringLength()));
  //      boost::match_results<std::string::const_iterator> mret;
  //      if (!boost::regex_search(data, mret, r_data)) {
  //        break;
  //      }

  //      auto data_type_iter = itr->value.FindMember("type");
  //      if (data_type_iter == itr->value.MemberEnd() &&
  //          !data_type_iter->value.IsString()) {
  //        break;
  //      }

  //      if (data_type_iter->value.IsString()) {
  //        data_type = std::string(data_type_iter->value.GetString(),
  //                                data_type_iter->value.GetStringLength());
  //      } else {
  //      }
  //    }

  //    if (itr->name.IsString()) {
  //      auto name =
  //          std::string(itr->name.GetString(), itr->name.GetStringLength());
  //      regex r_key("(?=key_)\\d+"), r_value("(?=value_)\\d+");
  //      boost::match_results<std::string::const_iterator> mret;
  //      if (boost::regex_search(name, mret, r_key)) {
  //        auto index = boost::lexical_cast<uint32>(mret[0].str());
  //      } else if (boost::regex_search(name, mret, r_value)) {
  //        auto index = boost::lexical_cast<uint32>(mret[0].str());
  //      } else {
  //      }
  //    } else {
  //    }
  //  }
  //}

  // impl_ptr->next();
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
