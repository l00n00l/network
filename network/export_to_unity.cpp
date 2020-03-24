#include "../utils/dicts.h"
#include "../utils/id_generator.h"
#include "../utils/regex_string.h"
#include "../utils/utils.h"
#include "server_mgr.h"
#include "session_mgr.h"
#include "tcp_proto.h"
#include <unordered_map>
#ifdef _WINDOWS
#define _DLLExport __declspec(dllexport)
#else
#define _DLLExport
#endif // _WINDOWS
extern dicts g_net_dicts;
extern "C" {
// log库
_DLLExport void regist_log_info_func(log_func f) { set_info_log_func(f); }
_DLLExport void regist_log_debug_func(log_func f) { set_debug_log_func(f); }
_DLLExport void regist_log_error_func(log_func f) { set_error_log_func(f); }

// 基本网络库
io_context *ioc_ptr = nullptr;
session_mgr *unity_session_mgr = nullptr;
_DLLExport bool init(char *proto_path) {
  try {
    if (!load_protos(proto_path)) {
      return false;
    }
    if (!ioc_ptr)
      ioc_ptr = new io_context;
    if (!unity_session_mgr)
      unity_session_mgr = new session_mgr(*ioc_ptr);
    return true;
  } catch (const std::exception &e) {
    lserr << __FUNCTION__ << " " >> std::string(e.what());
    return false;
  }
}
_DLLExport void run() {
  if (ioc_ptr)
    ioc_ptr->run_one();
}
_DLLExport void stop() {
  if (unity_session_mgr) {
    delete unity_session_mgr;
    unity_session_mgr = nullptr;
  }
  if (ioc_ptr) {
    delete ioc_ptr;
    ioc_ptr = nullptr;
  }
}
_DLLExport void disconnect(uint64 session_id) {
  if (unity_session_mgr)
    unity_session_mgr->remove_session(session_id);
}
_DLLExport uint64 connect_server(char *proto_name, char *host, char *port) {
  if (!unity_session_mgr)
    return 0;
  return unity_session_mgr->create_session(proto_name, host, port);
}
_DLLExport void send_msg(uint64 session_id, char *msg) {
  if (unity_session_mgr)
    unity_session_mgr->send_msg(session_id, std::string(msg));
}
_DLLExport void send_msg_by_dataid(uint64 session_id, uint64 data_id,
                                   char *msg_name) {
  if (unity_session_mgr) {
    unity_session_mgr->send_msg(session_id,
                                g_net_dicts.get_string(data_id, msg_name));
  }
}
_DLLExport bool session_valid(uint64 session_id) {
  if (!unity_session_mgr)
    return false;
  return unity_session_mgr->session_valid(session_id);
}

// 网络变量
_DLLExport uint64 gen_net_dict() { return g_net_dicts.gen(); }
_DLLExport void remove_net_dict(uint64 id) { g_net_dicts.remove(id); }
_DLLExport void remove_net_value(uint64 id, const char *key,
                                 std::size_t key_size) {

  g_net_dicts.remove_value(id, std::string(key, key_size));
}

_DLLExport bool net_var_is_int(uint64 id, const char *key_ptr,
                               std::size_t key_size) {
  auto key = std::string(key_ptr, key_size);
  return g_net_dicts.is_int8(id, key) || g_net_dicts.is_int16(id, key) ||
         g_net_dicts.is_int32(id, key) || g_net_dicts.is_int64(id, key);
}
_DLLExport bool net_var_is_uint(uint64 id, const char *key_ptr,
                                std::size_t key_size) {
  auto key = std::string(key_ptr, key_size);
  return g_net_dicts.is_uint8(id, key) || g_net_dicts.is_uint16(id, key) ||
         g_net_dicts.is_uint32(id, key) || g_net_dicts.is_uint64(id, key);
}

_DLLExport bool net_var_is_float(uint64 id, const char *key_ptr,
                                 std::size_t key_size) {
  auto key = std::string(key_ptr, key_size);
  return g_net_dicts.is_float32(id, key) || g_net_dicts.is_float64(id, key);
}

_DLLExport bool net_var_is_string(uint64 id, const char *key_ptr,
                                  std::size_t key_size) {
  auto key = std::string(key_ptr, key_size);
  return g_net_dicts.is_string(id, key);
}

_DLLExport int64 net_var_get_long(uint64 id, const char *key_ptr,
                                  std::size_t key_size) {
  auto key = std::string(key_ptr, key_size);
  if (g_net_dicts.is_int8(id, key)) {
    return g_net_dicts.get_int8(id, key);
  } else if (g_net_dicts.is_int16(id, key)) {
    return g_net_dicts.get_int16(id, key);
  } else if (g_net_dicts.is_int32(id, key)) {
    return g_net_dicts.get_int32(id, key);
  } else if (g_net_dicts.is_int64(id, key)) {
    return g_net_dicts.get_int64(id, key);
  }
  return 0;
}
_DLLExport uint64 net_var_get_ulong(uint64 id, const char *key_ptr,
                                    std::size_t key_size) {
  auto key = std::string(key_ptr, key_size);
  if (g_net_dicts.is_uint8(id, key)) {
    return g_net_dicts.get_uint8(id, key);
  } else if (g_net_dicts.is_uint16(id, key)) {
    return g_net_dicts.get_uint16(id, key);
  } else if (g_net_dicts.is_uint32(id, key)) {
    return g_net_dicts.get_uint32(id, key);
  } else if (g_net_dicts.is_uint64(id, key)) {
    return g_net_dicts.get_uint64(id, key);
  }
  return 0;
}

_DLLExport float64 net_var_get_float(uint64 id, const char *key_ptr,
                                     std::size_t key_size) {
  auto key = std::string(key_ptr, key_size);
  if (g_net_dicts.is_float32(id, key)) {
    return g_net_dicts.get_float32(id, key);
  } else if (g_net_dicts.is_float64(id, key)) {
    return g_net_dicts.get_float64(id, key);
  }
  return 0;
}

_DLLExport const char *net_var_get_string(uint64 id, const char *key_ptr,
                                          std::size_t key_size) {
  auto key = std::string(key_ptr, key_size);
  return g_net_dicts.get_string(id, key).c_str();
}
_DLLExport std::size_t net_var_get_string_size(uint64 id, const char *key_ptr,
                                               std::size_t key_size) {
  auto key = std::string(key_ptr, key_size);
  return g_net_dicts.get_string(id, key).size();
}

_DLLExport void net_var_set_long(uint64 id, const char *key_ptr,
                                 std::size_t key_size, int64 value) {
  auto key = std::string(key_ptr, key_size);
  g_net_dicts.set_int64(id, key, value);
}

_DLLExport void net_var_set_ulong(uint64 id, const char *key_ptr,
                                  std::size_t key_size, uint64 value) {
  auto key = std::string(key_ptr, key_size);
  g_net_dicts.set_int64(id, key, value);
}

_DLLExport void net_var_set_float(uint64 id, const char *key_ptr,
                                  std::size_t key_size, float64 value) {
  auto key = std::string(key_ptr, key_size);
  g_net_dicts.set_int64(id, key, value);
}
_DLLExport void net_var_set_string(uint64 id, const char *key_ptr,
                                   std::size_t key_size, const char *data_ptr,
                                   std::size_t data_size) {
  auto key = std::string(key_ptr, key_size);
  g_net_dicts.set_string(id, key, std::string(data_ptr, data_size));
}

_DLLExport void set_msg_handler(message_handler_type handler) {
  set_message_handler(handler);
}

std::unordered_map<uint64, dict_iterator> dict_iterator_map;
id_generator dict_iterator_id_gen;
std::atomic_flag dict_iterator_flag;
_DLLExport uint64 create_dict_iterator(uint64 data_id) {
  auto id = dict_iterator_id_gen.gen();
  dict_iterator itr(data_id, &g_net_dicts);
  atomic_flag_acquire(dict_iterator_flag);
  dict_iterator_map[id] = itr;
  atomic_flag_release(dict_iterator_flag);
  return id;
}

_DLLExport void remove_dict_iterator(uint64 id) {
  atomic_flag_acquire(dict_iterator_flag);
  dict_iterator_map.erase(id);
  atomic_flag_release(dict_iterator_flag);
  dict_iterator_id_gen.recycle(id);
}

_DLLExport const char *dict_iterator_get(uint64 id) {
  auto ret = "";
  atomic_flag_acquire(dict_iterator_flag);
  auto iter = dict_iterator_map.find(id);
  if (iter != dict_iterator_map.end()) {
    ret = iter->second().c_str();
  }
  atomic_flag_release(dict_iterator_flag);
  return ret;
}

std::unordered_map<uint64, regstring> regstring_map;
id_generator regstring_map_id_gen;
std::atomic_flag regstring_map_flag;
_DLLExport uint64 create_regstring() { return regstring_map_id_gen.gen(); }
_DLLExport void remove_regstring(uint64 id) {
  atomic_flag_acquire(regstring_map_flag);
  regstring_map.erase(id);
  atomic_flag_release(regstring_map_flag);
  regstring_map_id_gen.recycle(id);
}
_DLLExport bool regstring_parse_regex(uint64 id, const char *r,
                                      std::size_t rsize) {
  auto ret = false;
  atomic_flag_acquire(regstring_map_flag);
  auto iter = regstring_map.find(id);
  if (iter != regstring_map.end()) {
    ret = iter->second.parse_regex(r, rsize);
  }
  atomic_flag_release(regstring_map_flag);
  return ret;
}
_DLLExport void regstring_set(uint64 id, const char *name, const char *data,
                              std::size_t data_size) {
  atomic_flag_acquire(regstring_map_flag);
  auto iter = regstring_map.find(id);
  if (iter != regstring_map.end()) {
    iter->second.set(name, data, data_size);
  }
  atomic_flag_release(regstring_map_flag);
}
_DLLExport const char *regstring_get(uint64 id) {
  auto ret = "";
  atomic_flag_acquire(regstring_map_flag);
  auto iter = regstring_map.find(id);
  if (iter != regstring_map.end()) {
    ret = iter->second.c_str();
  }
  atomic_flag_release(regstring_map_flag);
  return ret;
}
_DLLExport bool make_message(const char *proto_name, int32 side, uint64 data_id,
                             const char *to_name) {
  std::stringstream ss;
  regstring rs;
  auto size = get_read_item_size(proto_name, side);
  std::string cache;
  for (std::size_t i = 0; i < size; i++) {
    if (!rs.parse_regex(get_compile_regex(proto_name, side, i))) {
      return false;
    }

    for (auto &name : rs.names()) {
      cache = g_net_dicts.get_string(data_id, name);
      rs.set(name.c_str(), cache.c_str(), cache.size());
    }
    ss << rs.str();
  }
  g_net_dicts.set_string(data_id, to_name, ss.str());
  return true;
}
}