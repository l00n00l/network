#include "conf.h"
#include "boost/filesystem.hpp"
#include "conf.h"
#include "json.h"
#include <atomic>

using namespace rapidjson;

namespace fsys = boost::filesystem;
#define conf_get(target_type)                                                  \
  auto ret = default_value;                                                    \
  atomic_flag_acquire(impl_ptr->wflag);                                        \
  auto iter = impl_ptr->document.FindMember(field_name.c_str());               \
  if (iter != impl_ptr->document.MemberEnd() &&                                \
      iter->value.Is##target_type()) {                                         \
    ret = iter->value.Get##target_type();                                      \
  }                                                                            \
  atomic_flag_release(impl_ptr->wflag);                                        \
  return ret;

#define conf_set                                                               \
  atomic_flag_acquire(impl_ptr->wflag);                                        \
  impl_ptr->document[field_name.c_str()] = value;                              \
  atomic_flag_release(impl_ptr->wflag);                                        \
  _write_file();

struct conf::impl {
  Document document;
  std::string filename;
  std::atomic_flag wflag;
  impl() { wflag.clear(); }
};

conf::conf() { impl_ptr = new impl; }

conf::~conf() {
  if (impl_ptr)
    delete impl_ptr;
}

void conf::init(const std::string &filename) {
  atomic_flag_acquire(impl_ptr->wflag);
  if (fsys::exists(filename) && fsys::is_regular_file(filename)) {
    impl_ptr->filename = filename;
    impl_ptr->document = json_load_fs(filename.c_str());
  } else {
    impl_ptr->document.Parse("{}");
  }
  atomic_flag_release(impl_ptr->wflag);
}

int32 conf::get_int32(const std::string field_name,
                      int32 default_value){conf_get(Int)}

int64 conf::get_int64(const std::string field_name,
                      int64 default_value){conf_get(Int64)}

uint32 conf::get_uint32(const std::string field_name,
                        uint32 default_value){conf_get(Uint)}

uint64 conf::get_uint64(const std::string field_name, uint64 default_value) {
  conf_get(Uint64)
}

const std::string conf::get_str(const std::string field_name,
                                const std::string &default_value) {
  auto ret = default_value;
  atomic_flag_acquire(impl_ptr->wflag);
  auto iter = impl_ptr->document.FindMember(field_name.c_str());
  if (iter != impl_ptr->document.MemberEnd()) {
    ret = std::string(iter->value.GetString(), iter->value.GetStringLength());
  }
  atomic_flag_release(impl_ptr->wflag);
  return ret;
}

void conf::set_int32(const std::string field_name, int32 value) { conf_set }

void conf::set_int64(const std::string field_name, int64 value) { conf_set }

void conf::set_uint32(const std::string field_name, uint32 value) { conf_set }

void conf::set_uint64(const std::string field_name, uint64 value) { conf_set }

void conf::set_str(const std::string field_name, const std::string &value) {
  atomic_flag_acquire(impl_ptr->wflag);
  impl_ptr->document[field_name.c_str()] =
      StringRef(value.c_str(), value.size());
  atomic_flag_release(impl_ptr->wflag);
  _write_file();
}

void conf::_write_file() {
  if (impl_ptr->filename.empty())
    return;
  json_write_fs(impl_ptr->document, impl_ptr->filename.c_str());
}
