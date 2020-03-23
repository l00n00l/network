#pragma once
#include "utils.h"
class dicts;
class dict_iterator {
public:
  dict_iterator(uint64 id, dicts *dict_ptr);
  ~dict_iterator();
  const std::string operator()();

private:
  struct dict_iterator_impl;
  dict_iterator_impl *impl_ptr;
};

class dicts {
public:
  dicts();
  ~dicts();

  uint64 gen();
  void remove(uint64 id);
  void remove_value(uint64 id, const std::string &key);

public:
  bool has_dict(uint64 id);
  bool has_value(uint64 id, const std::string &key);
  bool is_int8(uint64 id, const std::string &key);
  bool is_int16(uint64 id, const std::string &key);
  bool is_int32(uint64 id, const std::string &key);
  bool is_int64(uint64 id, const std::string &key);
  bool is_uint8(uint64 id, const std::string &key);
  bool is_uint16(uint64 id, const std::string &key);
  bool is_uint32(uint64 id, const std::string &key);
  bool is_uint64(uint64 id, const std::string &key);
  bool is_float32(uint64 id, const std::string &key);
  bool is_float64(uint64 id, const std::string &key);
  bool is_string(uint64 id, const std::string &key);
  bool is_char_ptr(uint64 id, const std::string &key);
  int8 get_int8(uint64 id, const std::string &key);
  int16 get_int16(uint64 id, const std::string &key);
  int32 get_int32(uint64 id, const std::string &key);
  int64 get_int64(uint64 id, const std::string &key);
  uint8 get_uint8(uint64 id, const std::string &key);
  uint16 get_uint16(uint64 id, const std::string &key);
  uint32 get_uint32(uint64 id, const std::string &key);
  uint64 get_uint64(uint64 id, const std::string &key);
  float32 get_float32(uint64 id, const std::string &key);
  float64 get_float64(uint64 id, const std::string &key);
  std::string get_string(uint64 id, const std::string &key);
  void build_c_str(uint64 id, const std::string &key, const char *&data_ptr,
                   std::size_t &size);
  void set_int8(uint64 id, const std::string &key, int8 const &value);
  void set_int16(uint64 id, const std::string &key, int16 const &value);
  void set_int32(uint64 id, const std::string &key, int32 const &value);
  void set_int64(uint64 id, const std::string &key, int64 const &value);
  void set_uint8(uint64 id, const std::string &key, uint8 const &value);
  void set_uint16(uint64 id, const std::string &key, uint16 const &value);
  void set_uint32(uint64 id, const std::string &key, uint32 const &value);
  void set_uint64(uint64 id, const std::string &key, uint64 const &value);
  void set_float32(uint64 id, const std::string &key, float32 const &value);
  void set_float64(uint64 id, const std::string &key, float64 const &value);
  void set_string(uint64 id, const std::string &key, std::string const &value);

private:
  friend struct dict_iterator;
  struct impl;
  impl *impl_ptr;
};