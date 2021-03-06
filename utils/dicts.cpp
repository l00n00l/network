#include "dicts.h"
#include "id_generator.h"
#include "log.h"
#include <boost/any.hpp>
#include <unordered_map>

typedef std::unordered_map<std::string, boost::any> dict_type;
typedef std::unordered_map<uint64, dict_type> dict_map_type;
struct dicts::impl {
  id_generator id_gen;
  dict_map_type dict_map;
  std::atomic_flag flag;
  impl() { flag.clear(); }
};

struct dict_iterator::dict_iterator_impl {
  dict_type::const_iterator iter;
  dict_type::const_iterator iter_end;
  std::string end_str;
};

dict_iterator::dict_iterator(uint64 id, dicts *dict_ptr) {
  impl_ptr = new dict_iterator_impl;
  if (dict_ptr) {
    auto iter = dict_ptr->impl_ptr->dict_map.find(id);
    if (iter != dict_ptr->impl_ptr->dict_map.end()) {
      impl_ptr->iter = iter->second.begin();
      impl_ptr->iter_end = iter->second.end();
    }
  }
}

dict_iterator::~dict_iterator() {
  if (impl_ptr) {
    delete impl_ptr;
  }
}

const std::string dict_iterator::operator()() {
  if (impl_ptr->iter != impl_ptr->iter_end) {
    auto ret = impl_ptr->iter->first;
    ++impl_ptr->iter;
    return ret;
  }

  return impl_ptr->end_str;
}

dicts::dicts() : impl_ptr(new impl) {}

dicts::~dicts() {
  if (impl_ptr)
    delete impl_ptr;
}

uint64 dicts::gen() { return impl_ptr->id_gen.gen(); }

void dicts::remove(uint64 id) {
  atomic_flag_acquire(impl_ptr->flag);
  impl_ptr->dict_map.erase(id);
  atomic_flag_release(impl_ptr->flag);
  impl_ptr->id_gen.recycle(id);
}

void dicts::remove_value(uint64 id, const std::string &key) {
  atomic_flag_acquire(impl_ptr->flag);
  auto iter = impl_ptr->dict_map.find(id);
  if (iter != impl_ptr->dict_map.end())
    iter->second.erase(key);
  atomic_flag_release(impl_ptr->flag);
}
#define is_type(t, id, key)                                                    \
  auto ret = false;                                                            \
  atomic_flag_acquire(impl_ptr->flag);                                         \
  auto iter = impl_ptr->dict_map.find(id);                                     \
  if (iter == impl_ptr->dict_map.end()) {                                      \
    ret = false;                                                               \
  } else {                                                                     \
    auto iter2 = iter->second.find(key);                                       \
    if (iter2 == iter->second.end()) {                                         \
      ret = false;                                                             \
    } else {                                                                   \
      ret = typeid(t) == iter2->second.type();                                 \
    }                                                                          \
  }                                                                            \
  atomic_flag_release(impl_ptr->flag);                                         \
  return ret;

bool dicts::has_dict(uint64 id) {
  auto ret = true;
  atomic_flag_acquire(impl_ptr->flag);
  auto iter = impl_ptr->dict_map.find(id);
  if (iter == impl_ptr->dict_map.end())
    ret = false;
  atomic_flag_release(impl_ptr->flag);
  return ret;
}

bool dicts::has_value(uint64 id, const std::string &key) {
  auto ret = true;
  atomic_flag_acquire(impl_ptr->flag);
  auto iter = impl_ptr->dict_map.find(id);
  if (iter == impl_ptr->dict_map.end()) {
    ret = false;
  } else {
    auto iter2 = iter->second.find(key);
    if (iter2 == iter->second.end()) {
      ret = false;
    }
  }
  atomic_flag_release(impl_ptr->flag);
  return ret;
}

bool dicts::is_int8(uint64 id, const std::string &key) {
  is_type(int8, id, key)
}

bool dicts::is_int16(uint64 id, const std::string &key) {
  is_type(int16, id, key)
}

bool dicts::is_int32(uint64 id, const std::string &key) {
  is_type(int32, id, key)
}

bool dicts::is_int64(uint64 id, const std::string &key) {
  is_type(int64, id, key)
}

bool dicts::is_uint8(uint64 id, const std::string &key) {
  is_type(uint8, id, key)
}

bool dicts::is_uint16(uint64 id, const std::string &key) {
  is_type(uint16, id, key)
}

bool dicts::is_uint32(uint64 id, const std::string &key) {
  is_type(uint32, id, key)
}

bool dicts::is_uint64(uint64 id, const std::string &key) {
  is_type(uint64, id, key)
}

bool dicts::is_float32(uint64 id, const std::string &key) {
  is_type(float32, id, key)
}

bool dicts::is_float64(uint64 id, const std::string &key) {
  is_type(float64, id, key)
}

bool dicts::is_string(uint64 id, const std::string &key) {
  auto ret = false;
  atomic_flag_acquire(impl_ptr->flag);
  auto iter = impl_ptr->dict_map.find(id);
  if (iter == impl_ptr->dict_map.end()) {
    ret = false;
  } else {
    auto iter2 = iter->second.find(key);
    if (iter2 == iter->second.end()) {
      ret = false;
    } else {
      ret = boost::any_cast<std::string>(&(iter2->second));
    }
  }
  atomic_flag_release(impl_ptr->flag);
  return ret;
}

bool dicts::is_char_ptr(uint64 id, const std::string &key) {
  auto ret = false;
  atomic_flag_acquire(impl_ptr->flag);
  auto iter = impl_ptr->dict_map.find(id);
  if (iter == impl_ptr->dict_map.end()) {
    ret = false;
  } else {
    auto iter2 = iter->second.find(key);
    if (iter2 == iter->second.end()) {
      ret = false;
    } else {
      try {
        boost::any_cast<const char *>(&(iter2->second));
        ret = true;
      } catch (const boost::bad_any_cast &) {
        ret = false;
      }
    }
  }
  atomic_flag_release(impl_ptr->flag);
  return ret;
}
#define get_value(t, id, key)                                                  \
  t ret = 0;                                                                   \
  atomic_flag_acquire(impl_ptr->flag);                                         \
  auto iter = impl_ptr->dict_map.find(id);                                     \
  if (iter != impl_ptr->dict_map.end()) {                                      \
    auto iter2 = iter->second.find(key);                                       \
    if (iter2 != iter->second.end()) {                                         \
      try {                                                                    \
        ret = *boost::any_cast<t>(&(iter2->second));                           \
      } catch (const boost::bad_any_cast &) {                                  \
        lserr << u8"数据转换错误，目标类型不是：" << #t << "!" >>              \
            __FUNCTION__;                                                      \
      }                                                                        \
    }                                                                          \
  }                                                                            \
  atomic_flag_release(impl_ptr->flag);                                         \
  return ret;

int8 dicts::get_int8(uint64 id,
                     const std::string &key){get_value(int8, id, key)}

int16 dicts::get_int16(uint64 id,
                       const std::string &key){get_value(int16, id, key)}

int32 dicts::get_int32(uint64 id,
                       const std::string &key){get_value(int32, id, key)}

int64 dicts::get_int64(uint64 id,
                       const std::string &key){get_value(int64, id, key)}

uint8 dicts::get_uint8(uint64 id,
                       const std::string &key){get_value(uint8, id, key)}

uint16 dicts::get_uint16(uint64 id,
                         const std::string &key){get_value(uint16, id, key)}

uint32 dicts::get_uint32(uint64 id,
                         const std::string &key){get_value(uint32, id, key)}

uint64 dicts::get_uint64(uint64 id,
                         const std::string &key){get_value(uint64, id, key)}

float32 dicts::get_float32(uint64 id,
                           const std::string &key){get_value(float32, id, key)}

float64 dicts::get_float64(uint64 id,
                           const std::string &key){get_value(float64, id, key)}

std::string dicts::get_string(uint64 id, const std::string &key) {
  std::string ret;
  atomic_flag_acquire(impl_ptr->flag);
  auto iter = impl_ptr->dict_map.find(id);
  if (iter != impl_ptr->dict_map.end()) {
    auto iter2 = iter->second.find(key);
    if (iter2 != iter->second.end()) {
      try {
        ret = *boost::any_cast<std::string>(&(iter2->second));
      } catch (const boost::bad_any_cast &) {
        lserr << u8"数据转换错误，目标类型不是：string!" >> __FUNCTION__;
      }
    }
  }
  atomic_flag_release(impl_ptr->flag);
  return ret;
}

void dicts::build_c_str(uint64 id, const std::string &key,
                        const char *&data_ptr, std::size_t &size) {
  atomic_flag_acquire(impl_ptr->flag);
  auto iter = impl_ptr->dict_map.find(id);
  if (iter != impl_ptr->dict_map.end()) {
    auto iter2 = iter->second.find(key);
    if (iter2 == iter->second.end()) {
      try {
        auto data = boost::any_cast<std::string>(&(iter2->second));
        data_ptr = data->c_str();
        size = data->size();
      } catch (const boost::bad_any_cast &) {
        lserr << u8"数据转换错误，目标类型不是：string!" >> __FUNCTION__;
      }
    }
  }
  atomic_flag_release(impl_ptr->flag);
}
template <typename T, typename D>
inline void set_value(D *impl_ptr, uint64 id, const std::string &key,
                      T const &value) {
  atomic_flag_acquire(impl_ptr->flag);
  impl_ptr->dict_map[id][key] = value;
  atomic_flag_release(impl_ptr->flag);
}

void dicts::set_int8(uint64 id, const std::string &key, int8 const &value) {
  set_value(impl_ptr, id, key, value);
}

void dicts::set_int16(uint64 id, const std::string &key, int16 const &value) {
  set_value(impl_ptr, id, key, value);
}

void dicts::set_int32(uint64 id, const std::string &key, int32 const &value) {
  set_value(impl_ptr, id, key, value);
}

void dicts::set_int64(uint64 id, const std::string &key, int64 const &value) {
  set_value(impl_ptr, id, key, value);
}

void dicts::set_uint8(uint64 id, const std::string &key, uint8 const &value) {
  set_value(impl_ptr, id, key, value);
}

void dicts::set_uint16(uint64 id, const std::string &key, uint16 const &value) {
  set_value(impl_ptr, id, key, value);
}

void dicts::set_uint32(uint64 id, const std::string &key, uint32 const &value) {
  set_value(impl_ptr, id, key, value);
}

void dicts::set_uint64(uint64 id, const std::string &key, uint64 const &value) {
  set_value(impl_ptr, id, key, value);
}

void dicts::set_float32(uint64 id, const std::string &key,
                        float32 const &value) {
  set_value(impl_ptr, id, key, value);
}

void dicts::set_float64(uint64 id, const std::string &key,
                        float64 const &value) {
  set_value(impl_ptr, id, key, value);
}

void dicts::set_string(uint64 id, const std::string &key,
                       std::string const &value) {
  set_value(impl_ptr, id, key, value);
}
