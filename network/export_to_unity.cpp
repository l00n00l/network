#include "../utils/dicts.h"
#include "../utils/utils.h"
#include "server_mgr.h"
#include "session_mgr.h"
#ifdef _WINDOWS
#define _DLLExport __declspec(dllexport)
#else
#define _DLLExport
#endif // _WINDOWS
extern dicts g_net_dicts;

extern "C" {
// 基本网络库
io_context *ioc_ptr = nullptr;
session_mgr *unity_session_mgr = nullptr;
_DLLExport void init() {
  if (!ioc_ptr)
    ioc_ptr = new io_context;
  if (!unity_session_mgr)
    unity_session_mgr = new session_mgr(*ioc_ptr);
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
_DLLExport uint64 connect_server(char *host, char *port) {
  if (!unity_session_mgr)
    return 0;
  return unity_session_mgr->create_session(
      std::string("control"), std::string(host), std::string(port));
}
_DLLExport void send_msg(uint64 session_id, char *msg) {
  if (unity_session_mgr)
    unity_session_mgr->send_msg(session_id, std::string(msg));
}

_DLLExport bool session_valid(uint64 session_id) {
  if (!unity_session_mgr)
    return false;
  return unity_session_mgr->session_valid(session_id);
}

// 网络变量
_DLLExport uint64 gen_net_dict() { return g_net_dicts.gen(); }
_DLLExport void remove_net_dict(uint64 id) { g_net_dicts.remove(id); }
_DLLExport void remove_net_value(uint64 id, const std::string &key) {
  g_net_dicts.remove_value(id, key);
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
  const char *data_ptr = nullptr;
  std::size_t size = 0;
  g_net_dicts.build_c_str(id, key, data_ptr, size);
  return data_ptr;
}
}