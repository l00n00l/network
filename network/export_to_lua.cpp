#pragma once
#include "export_to_lua.h"
#include "server_mgr.h"
#include "session_mgr.h"
#include "tcp_proto.h"

using namespace luabridge;

server_mgr *get_server_mgr() { return &(*g_server_mgr); }
session_mgr *get_session_mgr() { return &(*g_session_mgr); }

void export_net(lua_State *L) {
  getGlobalNamespace(L)
      .addProperty("g_server_mgr", get_server_mgr)
      .addProperty("g_session_mgr", get_session_mgr)
      .addFunction("load_protos", load_protos)
      .beginClass<session_mgr>("session_mgr")
      .addFunction("connect_to", &session_mgr::connect_to)
      .addFunction("remove_session", &session_mgr::remove_session)
      .addFunction("session_valid", &session_mgr::session_valid)
      .addFunction("send_msg", &session_mgr::send_msg)
      .endClass()
      .beginClass<server_mgr>("server_mgr")
      .addFunction("create_server", &server_mgr::create_server)
      .addFunction("destroy_server", &server_mgr::destroy_server)
      .endClass()
      .endNamespace();
}
