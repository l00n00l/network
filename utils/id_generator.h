#pragma once

#include "utils.h"

class id_generator {
public:
  id_generator();
  ~id_generator();

  uint64 gen();
  void recycle(uint64 value);

private:
  struct id_generator_impl;
  std::unique_ptr<id_generator_impl> impl_ptr;
};
