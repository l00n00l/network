#pragma once

#include "utils.h"
#include <atomic>
#include <functional>
#include <list>

template <typename data_type> class lock_free_queue {
public:
  typedef std::function<void(data_type &)> consume_handler;

public:
  lock_free_queue() { _lock.clear(); }
  ~lock_free_queue() {}

public:
  bool push(data_type &data) {
    try {
      atomic_flag_acquire(_lock);
      _data.push_back(data);
      atomic_flag_release(_lock);
    } catch (const std::exception &) {
      atomic_flag_release(_lock);
      return false;
    }
    return true;
  }

  bool push(data_type &&data) {
    try {
      atomic_flag_acquire(_lock);
      _data.push_back(data);
      atomic_flag_release(_lock);
    } catch (const std::exception &) {
      // ÄÚ´æ²»×ã
      atomic_flag_release(_lock);
      return false;
    }
    return true;
  }

  void pop(data_type &data) {
    atomic_flag_acquire(_lock);
    if (_data.size() <= 0) {
      return;
    }
    data = std::move(_data.front());
    _data.pop_front();
    atomic_flag_release(_lock);
  }

  void consume_one(consume_handler &&handler) {
    atomic_flag_acquire(_lock);
    if (_data.size() > 0) {
      handler(_data.front());
      _data.pop_front();
    }
    atomic_flag_release(_lock);
  }

  void consume_all(consume_handler &&handler) {
    atomic_flag_acquire(_lock);
    while (_data.size() > 0) {
      handler(_data.front());
      _data.pop_front();
    }
    atomic_flag_release(_lock);
  }

  std::size_t size() {
    atomic_flag_acquire(_lock);
    auto ret = _data.size();
    atomic_flag_release(_lock);
    return ret;
  }

  void for_each(consume_handler &&handler) {
    atomic_flag_acquire(_lock);
    for (auto &i : _data) {
      handler(i);
    }
    atomic_flag_release(_lock);
  }

  bool empty() { return size() <= 0; }

  void clear() {
    atomic_flag_acquire(_lock);
    _data.clear();
    atomic_flag_release(_lock);
  }

private:
  std::list<data_type> _data;
  std::atomic_flag _lock;
};