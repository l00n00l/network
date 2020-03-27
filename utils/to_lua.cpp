#include "to_lua.h"
#include "regex_string.h"
#include <boost/python.hpp>

using namespace luabridge;

inline std::string from_utf_(const std::string &s,
                             const std::string &locale_name) {
  return from_utf(s, locale_name);
}

inline std::string to_utf_(std::string const &s,
                           std::string const &locale_name) {
  return to_utf<char>(s, locale_name);
}

inline std::string utf_to_utf_(std::string const &s) {
  return utf_to_utf<char>(s);
}

struct atomic_flag {
  atomic_flag() { flag.clear(); }
  ~atomic_flag() { flag.clear(); }

  bool acquire() { return flag.test_and_set(std::memory_order_acquire); }
  void release() { flag.clear(std::memory_order_release); }
  bool test_and_set() { return flag.test_and_set(); }
  void clear() { flag.clear(); }

private:
  std::atomic_flag flag;
};

void export_utils(lua_State *L) {
  getGlobalNamespace(L)
      .addFunction("from_utf", from_utf_)
      .addFunction("to_utf", to_utf_)
      .addFunction("utf_to_utf", utf_to_utf_)
      .beginClass<atomic_flag>("atomic_flag")
      .addConstructor<void (*)(void)>()
      .addFunction("acquire", &atomic_flag::acquire)
      .addFunction("release", &atomic_flag::release)
      .addFunction("test_and_set", &atomic_flag::test_and_set)
      .addFunction("clear", &atomic_flag::clear)
      .endClass()
      .endNamespace();

  auto log_info_ref = getGlobal(L, "log_info");
  auto log_debug_ref = getGlobal(L, "log_debug");
  auto log_error_ref = getGlobal(L, "log_error");

  set_info_log_func([&](const char *s, std::size_t size) {
    if (!log_info_ref.isNil() && log_info_ref.isFunction()) {
      log_info_ref(std::string(s, size));
    }
  });
  set_debug_log_func([&](const char *s, std::size_t size) {
    if (!log_debug_ref.isNil() && log_debug_ref.isFunction()) {
      log_debug_ref(std::string(s, size));
    }
  });
  set_error_log_func([&](const char *s, std::size_t size) {
    if (!log_error_ref.isNil() && log_error_ref.isFunction()) {
      log_error_ref(std::string(s, size));
    }
  });
}

struct lua_regex {
  uint32 parse(const std::string &data) { return r.set_expression(data); }
  bool match(const std::string &data) { return regex_match(data, r); }
  bool search(const std::string &data) { return regex_search(data, mret, r); }
  std::string replace(const std::string &data, const std::string &fmt) {
    return regex_replace(data, r, fmt);
  }
  std::string iget(std::size_t index) { return mret[index]; }
  std::string sget(const std::string &name) { return mret[name]; }
  std::size_t ret_size() { return mret.size(); }

private:
  regex r;
  match_results_str mret;
};

void export_regex(lua_State *L) {
  getGlobalNamespace(L)
      .beginClass<lua_regex>("regex")
      .addFunction("parse", &lua_regex::parse)
      .addFunction("match", &lua_regex::match)
      .addFunction("search", &lua_regex::search)
      .addFunction("replace", &lua_regex::replace)
      .addFunction("iget", &lua_regex::iget)
      .addFunction("sget", &lua_regex::sget)
      .addFunction("ret_size", &lua_regex::ret_size)
      .endClass()
      .endNamespace();
}

void export_network(lua_State *L) {
  getGlobalNamespace(L)
      .beginClass<io_context>("io_context")
      .addConstructor<void (*)(void)>()
      .endClass()
      .beginClass<tcp>("tcp")
      .addStaticFunction("v4", &tcp::v4)
      .addStaticFunction("v6", &tcp::v6)
      .endClass()
      .beginClass<tcp::endpoint>("tcp_endpoint")
      .addConstructor<void (*)(tcp &, uint16)>()
      .endClass()
      .endNamespace();
}

void export_regstring(lua_State *L) {
  getGlobalNamespace(L)
      .beginClass<regstring>("regstring")
      .addConstructor<void (*)(void)>()
      .addFunction("parse_regex", &regstring::parse_regex)
      .addFunction("set", &regstring::set)
      .addFunction("str", &regstring::str)
      .addFunction("size", &regstring::size)
      .endClass()
      .endNamespace();
}
void export_python(lua_State *L) {
  getGlobalNamespace(L)
      .beginClass<boost::python::object>("pyobj")
      .addConstructor<void (*)(void)>()
      .endClass()
      .endNamespace();
}

void utils_export(lua_State *L) {
  export_utils(L);
  export_regex(L);
  export_network(L);
  export_regstring(L);
}
