#include "../utils/utils.h"
#include "server_mgr.h"
#include "session_mgr.h"

//#define _DLLExport __declspec(dllexport)
extern "C" {
io_context *ioc_ptr = nullptr;
session_mgr *unity_session_mgr = nullptr;
void init() {
  if (!ioc_ptr)
    ioc_ptr = new io_context;
  if (!unity_session_mgr)
    unity_session_mgr = new session_mgr(*ioc_ptr);
}
void run() {
  if (ioc_ptr)
    ioc_ptr->run_one();
}
void stop() {
  if (unity_session_mgr) {
    delete unity_session_mgr;
    unity_session_mgr = nullptr;
  }
  if (ioc_ptr) {
    delete ioc_ptr;
    ioc_ptr = nullptr;
  }
}
void disconnect(uint64 session_id) {
  if (unity_session_mgr)
    unity_session_mgr->remove_session(session_id);
}
uint64 connect_server(char *host, char *port) {
  if (!unity_session_mgr)
    return 0;
  return unity_session_mgr->create_session(
      std::string("control"), std::string(host), std::string(port));
}
void send_msg(uint64 session_id, char *msg) {
  if (unity_session_mgr)
    unity_session_mgr->send_msg(session_id, std::string(msg));
}

bool session_valid(uint64 session_id) {
  if (!unity_session_mgr)
    return false;
  return unity_session_mgr->session_valid(session_id);
}
}