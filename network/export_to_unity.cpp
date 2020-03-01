#include "../utils/utils.h"
#include "server_mgr.h"
#include "session_mgr.h"

#define _DLLExport __declspec(dllexport)

extern "C" {
_DLLExport io_context *ioc_ptr = nullptr;
_DLLExport session_mgr *unity_session_mgr = nullptr;
_DLLExport void init() {
  if (!ioc_ptr)
    ioc_ptr = new io_context;
  if (!unity_session_mgr)
    unity_session_mgr = new session_mgr(*ioc_ptr);
}
_DLLExport void run() { ioc_ptr->run_one(); }
_DLLExport void stop() {
  delete unity_session_mgr;
  unity_session_mgr = nullptr;
  delete ioc_ptr;
  ioc_ptr = nullptr;
}
_DLLExport void disconnect(uint64 session_id) {
  unity_session_mgr->remove_session(session_id);
}
_DLLExport uint64 connect_server(char *host, char *port) {
  return unity_session_mgr->create_session(
      std::string("control"), std::string(host), std::string(port));
}
_DLLExport void send_msg(uint64 session_id, char *msg) {
  unity_session_mgr->send_msg(session_id, std::string(msg));
}

_DLLExport bool session_valid(uint64 session_id) {
  return unity_session_mgr->session_valid(session_id);
}
}