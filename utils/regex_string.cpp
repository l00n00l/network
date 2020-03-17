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
      : type(��ͨ�ַ�), count(1), close_flag(false), or_flag(false) {}
  void reset() {
    data = "";
    name = "";
    type = ��ͨ�ַ�;
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

// �Ƿ����, �Ƿ�ı����壬������
inline std::tuple<bool, bool, regex_type> test_close(regex_type type,
                                                     const char c) {
  switch (type) {
  case ��ͨ�ַ�:
    switch (c) {
    case '[':
      return std::make_tuple(true, true, ����);
    case '{':
      return std::make_tuple(true, true, �ظ�����1);
    case '(':
      return std::make_tuple(true, true, ����);
    }
    break;
  case �����ʺ�:
    switch (c) {
    case '<':
      return std::make_tuple(true, true, ���պ�����);
    case '=':
      return std::make_tuple(true, true, ��հ);
    case '!':
      return std::make_tuple(true, true, �񶨺�հ);
    case '#':
      return std::make_tuple(true, true, ע��);
    }
    break;
  case ����:
    switch (c) {
    case '^':
      return std::make_tuple(true, true, �񶨼���);
    case ']':
      return std::make_tuple(true, true, �ȴ��ظ�1);
    }
    break;
  case �񶨼���:
    if (c == ']')
      return std::make_tuple(true, true, �ȴ��ظ�1);
    break;
  case �ظ�����:
    if (c == '}')
      return std::make_tuple(true, false, ��ͨ�ַ�);
    break;
  case �ظ�����1:
    switch (c) {
    case ',':
      return std::make_tuple(true, true, �ظ�����);
    case '}':
      return std::make_tuple(true, false, ��ͨ�ַ�);
    }
    break;
  case ����:
    switch (c) {
    case '?':
      return std::make_tuple(true, true, �����ʺ�);
    case '|':
      return std::make_tuple(true, true, ����);
    case ')':
      return std::make_tuple(true, true, �ȴ��ظ�1);
    }
    break;
  case ����:
    switch (c) {
    case '>':
      return std::make_tuple(true, true, ����);
    }
    break;
  case ����:
  case ǰհ:
  case ��ǰհ:
  case ��հ:
  case �񶨺�հ:
  case �ǲ�����ʽ:
  case ע��:
    if (c == ')')
      return std::make_tuple(true, true, �ȴ��ظ�1);
    break;
  case ���պ�����:
    if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
        (c >= '0' && c <= '9') || c == '_') {
      return std::make_tuple(true, true, ����);
    } else {
      switch (c) {
      case '=':
        return std::make_tuple(true, true, ǰհ);
      case '!':
        return std::make_tuple(true, true, ��ǰհ);
      }
    }
    break;
  case �ȴ��ظ�1:
    switch (c) {
    case '*':
    case '+':
      return std::make_tuple(true, true, �ȴ��ظ�2);
    default:
      return std::make_tuple(true, true, ��ͨ�ַ�);
    }
    break;
  case �ȴ��ظ�2:
    switch (c) {
    case '?':
      return std::make_tuple(true, true, ��ͨ�ַ�);
    }
    break;
  }

  return std::make_tuple(false, false, ��ͨ�ַ�);
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
    lserr << u8"ǰ�溯����������,"
             u8"�����Ա�����룬վ�ڲ����ܳ��ֱպϳ���������" >>
        __FUNCTION__;
    return std::make_tuple(false, false);
  }

  // ȫ���պ����
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
  // ���Ապϵ�ǰ����
  auto ret = test_close(cur_cache.type, c);
  // �պϵ�ǰ����ʧ��
  if (!std::get<0>(ret)) {
    // ���Ապϻ���ջ
    auto ret = try_close_on_queue(cache, cq, c);

    // ջ�պϳɹ�
    if (std::get<0>(ret)) {
      // ջ�Ƿ�����
      if (std::get<1>(ret)) {
        // ���ջ��ѵ�ǰ����Ҳ���뵽����ֵ
        for (uint32_t i = 0; i < cur_cache.count; i++) {
          regex_string_value temp(cur_cache.name, cur_cache.data);
          cache.push_back(temp);
        }
      } else {
        // ջ������գ��ѵ�ǰ������ջ
        cq.push_front(cur_cache);
      }

      // ��յ�ǰ����
      cur_cache.reset();
    }
    // ջ�պ�ʧ�ܣ����ַ����뵱ǰ����
    else {
      switch (cur_cache.type) {
      case ����:
        cur_cache.name += c;
        break;
      case ����:
      case �񶨼���:
        if (cur_cache.data.size() <= 0)
          cur_cache.data += c;
        break;
      case �ظ�����:
      case �ظ�����1: {
        std::stringstream ss;
        ss << c;
        cur_cache.count = std::atoi(ss.str().c_str());
      } break;
      case �ȴ��ظ�2:
        push_regex(cache, cq, cur_cache);
        cur_cache.reset();
        break;
      default:
        cur_cache.data += c;
        break;
      }
    }
  }
  // �պϵ�ǰ����ɹ�
  else {
    cur_cache.close_flag = true;
    if (std::get<1>(ret)) {
      auto new_type = std::get<2>(ret);
      if (new_type == ��ͨ�ַ�) {
        push_regex(cache, cq, cur_cache);
      } else {
        cur_cache.type = new_type;
        cur_cache.close_flag = false;
      }
      switch (cur_cache.type) {
      case ����:
        cur_cache.name += c;
        break;
      case ��ͨ�ַ�:
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
  case ��ͨ�ַ�:
  case ����:
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
