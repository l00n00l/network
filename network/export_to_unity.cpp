#include "../utils/utils.h"
#include "server_mgr.h"
#include "session_mgr.h"
#ifdef _WINDOWS
#define _DLLExport __declspec(dllexport)
#else
#define _DLLExport
#endif // _WINDOWS

extern "C" {
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
}