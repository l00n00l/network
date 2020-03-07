#pragma once
#include "utils.h"

class conf {
public:
  conf();
  ~conf();

  void init(const std::string &filename);
  int32 get_int32(const std::string field_name, int32 default_value);
  int64 get_int64(const std::string field_name, int64 default_value);
  uint32 get_uint32(const std::string field_name, uint32 default_value);
  uint64 get_uint64(const std::string field_name, uint64 default_value);
  const std::string get_str(const std::string field_name,
                            const std::string &default_value);
  void set_int32(const std::string field_name, int32 value);
  void set_int64(const std::string field_name, int64 value);
  void set_uint32(const std::string field_name, uint32 value);
  void set_uint64(const std::string field_name, uint64 value);
  void set_str(const std::string field_name, const std::string &value);

private:
  void _write_file();

private:
  struct impl;
  impl *impl_ptr;
};