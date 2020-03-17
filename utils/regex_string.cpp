#include "regex_string.h"
#include "log.h"
#include <map>
#include <sstream>
#include <tuple>
typedef std::map<const char, const char> str_map;

str_map special_str_{{'w', 'a'},  {'W', '.'},  {'s', '\r'}, {'S', 'a'},
                     {'d', '1'},  {'D', 'a'},  {'l', 'a'},  {'u', 'A'},
                     {'a', '\a'}, {'f', '\f'}, {'n', '\n'}, {'r', '\r'},
                     {'t', '\t'}, {'v', '\v'}, {'(', '('},  {')', ')'},
                     {'{', '{'},  {'}', '}'},  {'[', '['},  {']', ']'},
                     {'.', '.'},  {'|', '|'},  {'*', '*'},  {'?', '?'},
                     {'.', '.'},  {'|', '|'},  {'*', '*'},  {'?', '?'},
                     {'+', '+'},  {'^', '^'},  {'$', '$'}};

inline auto get_spec_str(const char c) {
  auto iter = special_str_.find(c);
  if (iter != special_str_.end()) {
    return iter->second;
  }
  return c;
}

struct regex_string_cache {
  std::string data;
  std::string name;
  regex_type type;
  uint32_t count;
  bool close_flag;
  bool or_flag;
  regex_string_cache()
      : type(普通字符), count(1), close_flag(false), or_flag(false) {}
  void reset() {
    data = "";
    name = "";
    type = 普通字符;
    count = 1;
    close_flag = false;
    or_flag = false;
  }
};

typedef std::list<regex_string_cache> cache_queue;

inline void push_regex(regex_string &cache, cache_queue &cq,
                       regex_string_cache &cur_cache) {
  if (cq.size() <= 0) {
    for (size_t i = 0; i < cur_cache.count; i++) {
      regex_string_value temp(cur_cache.data, cur_cache.name);
      cache.push_back(temp);
    }
  } else {
    cq.push_front(cur_cache);
  }
  cur_cache.reset();
}

// 是否错误, 是否改变语义，新语义
inline std::tuple<bool, bool, regex_type> test_close(regex_type type,
                                                     const char c) {
  switch (type) {
  case 普通字符:
    switch (c) {
    case '[':
      return std::make_tuple(true, true, 集合);
    case '{':
      return std::make_tuple(true, true, 重复次数1);
    case '(':
      return std::make_tuple(true, true, 括号);
    }
    break;
  case 括号问号:
    switch (c) {
    case '<':
      return std::make_tuple(true, true, 待闭合名称);
    case '=':
      return std::make_tuple(true, true, 后瞻);
    case '!':
      return std::make_tuple(true, true, 否定后瞻);
    case '#':
      return std::make_tuple(true, true, 注释);
    }
    break;
  case 集合:
    switch (c) {
    case '^':
      return std::make_tuple(true, true, 否定集合);
    case ']':
      return std::make_tuple(true, true, 等待重复1);
    }
    break;
  case 否定集合:
    if (c == ']')
      return std::make_tuple(true, true, 等待重复1);
    break;
  case 重复次数:
    if (c == '}')
      return std::make_tuple(true, false, 普通字符);
    break;
  case 重复次数1:
    switch (c) {
    case ',':
      return std::make_tuple(true, true, 重复次数);
    case '}':
      return std::make_tuple(true, false, 普通字符);
    }
    break;
  case 括号:
    switch (c) {
    case '?':
      return std::make_tuple(true, true, 括号问号);
    case '|':
      return std::make_tuple(true, true, 或者);
    case ')':
      return std::make_tuple(true, true, 等待重复1);
    }
    break;
  case 名称:
    switch (c) {
    case '>':
      return std::make_tuple(true, true, 括号);
    }
    break;
  case 或者:
  case 前瞻:
  case 否定前瞻:
  case 后瞻:
  case 否定后瞻:
  case 非捕获表达式:
  case 注释:
    if (c == ')')
      return std::make_tuple(true, true, 等待重复1);
    break;
  case 待闭合名称:
    if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
        (c >= '0' && c <= '9') || c == '_') {
      return std::make_tuple(true, true, 名称);
    } else {
      switch (c) {
      case '=':
        return std::make_tuple(true, true, 前瞻);
      case '!':
        return std::make_tuple(true, true, 否定前瞻);
      }
    }
    break;
  case 等待重复1:
    switch (c) {
    case '*':
    case '+':
      return std::make_tuple(true, true, 等待重复2);
    default:
      return std::make_tuple(true, true, 普通字符);
    }
    break;
  case 等待重复2:
    switch (c) {
    case '?':
      return std::make_tuple(true, true, 普通字符);
    }
    break;
  }

  return std::make_tuple(false, false, 普通字符);
}

inline std::tuple<bool, bool>
try_close_on_queue(regex_string &cache, cache_queue &cq, const char c) {
  if (cq.size() <= 0)
    return std::make_tuple(false, false);

  auto iter = cq.begin();
  while (!iter->close_flag) {
    ++iter;
    if (iter == cq.end()) {
      return std::make_tuple(false, false);
    }
  }

  auto ret = test_close(iter->type, c);
  if (!std::get<0>(ret))
    return std::make_tuple(false, false);

  iter->close_flag = true;

  if (std::get<1>(ret)) {
    lserr << u8"前面函数操作错误,"
             u8"请程序员检查代码，站内不可能出现闭合出现新语义" >>
        __FUNCTION__;
    return std::make_tuple(false, false);
  }

  // 全部闭合完成
  bool finished = iter == cq.end();
  if (finished) {
    for (auto itr = cq.rbegin(); itr != cq.rend(); ++itr) {
      for (uint32_t i = 0; i < itr->count; i++) {
        regex_string_value temp(itr->name, itr->data);
        cache.push_back(temp);
      }
    }
  }

  return std::make_tuple(true, finished);
}

inline bool push_cache(regex_string_cache &cur_cache, regex_string &cache,
                       cache_queue &cq, const char c) {
  // 尝试闭合当前缓存
  auto ret = test_close(cur_cache.type, c);
  // 闭合当前缓存失败
  if (!std::get<0>(ret)) {
    // 尝试闭合缓存栈
    auto ret = try_close_on_queue(cache, cq, c);

    // 栈闭合成功
    if (std::get<0>(ret)) {
      // 栈是否可清空
      if (std::get<1>(ret)) {
        // 清空栈后把当前缓存也加入到返回值
        for (uint32_t i = 0; i < cur_cache.count; i++) {
          regex_string_value temp(cur_cache.name, cur_cache.data);
          cache.push_back(temp);
        }
      } else {
        // 栈不可清空，把当前缓存入栈
        cq.push_front(cur_cache);
      }

      // 清空当前缓存
      cur_cache.reset();
    }
    // 栈闭合失败，将字符加入当前缓存
    else {
      switch (cur_cache.type) {
      case 名称:
        cur_cache.name += c;
        break;
      case 集合:
      case 否定集合:
        if (cur_cache.data.size() <= 0)
          cur_cache.data += c;
        break;
      case 重复次数:
      case 重复次数1: {
        std::stringstream ss;
        ss << c;
        cur_cache.count = std::atoi(ss.str().c_str());
      } break;
      case 等待重复2:
        push_regex(cache, cq, cur_cache);
        cur_cache.reset();
        break;
      default:
        cur_cache.data += c;
        break;
      }
    }
  }
  // 闭合当前缓存成功
  else {
    cur_cache.close_flag = true;
    if (std::get<1>(ret)) {
      auto new_type = std::get<2>(ret);
      if (new_type == 普通字符) {
        push_regex(cache, cq, cur_cache);
      } else {
        cur_cache.type = new_type;
        cur_cache.close_flag = false;
      }
      switch (cur_cache.type) {
      case 名称:
        cur_cache.name += c;
        break;
      case 普通字符:
        cur_cache.data += c;
        break;
      }
    } else {
      push_regex(cache, cq, cur_cache);
    }
  }

  return true;
}

inline void jump_repeat_char(const char *regex_ptr, std::size_t size,
                             uint32_t &cur_index) {
  auto next_index = cur_index + 1;
  if (next_index < size && regex_ptr[next_index] == '*' ||
      regex_ptr[next_index] == '+') {
    cur_index = next_index;
    next_index = cur_index + 1;
    if (next_index < size && regex_ptr[next_index] == '?') {
      cur_index = next_index;
    }
  }
}

bool read_regex(const char *regex_ptr, std::size_t size, uint32_t &cur_index,
                regex_string &cache, cache_queue &cq,
                regex_string_cache &cur_cache) {
  auto cur_char = regex_ptr[cur_index];
  switch (cur_char) {
  case '\\':
    if (++cur_index < size)
      cur_char = get_spec_str(regex_ptr[cur_index]);
    break;
  }
  switch (cur_cache.type) {
  case 普通字符:
  case 括号:
    jump_repeat_char(regex_ptr, size, cur_index);
    break;
  }

  push_cache(cur_cache, cache, cq, cur_char);

  if (++cur_index >= size) {
    push_regex(cache, cq, cur_cache);
    return true;
  }

  read_regex(regex_ptr, size, cur_index, cache, cq, cur_cache);
}

regex_string generate_regex_string(const char *regex_ptr, std::size_t size) {
  uint32_t index = 0;
  regex_string ret;
  cache_queue cq;
  regex_string_cache cur_cache;
  read_regex(regex_ptr, size, index, ret, cq, cur_cache);
  return ret;
}
