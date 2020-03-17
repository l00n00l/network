#pragma once
#include <cstddef>
#include <list>
#include <string>
enum regex_type {
  普通字符 = 1, // 普通字符
  集合,         // [char] // 不支持范围-
  否定集合,     // [^char]// 不支持范围-
  重复次数,     // {n1,n2}
  重复次数1,    // {n1}
  括号,         // (exp)
  或者,         // (exp|exp)
  名称,         //（?<name>exp)
  待闭合名称,   //（?<
  前瞻,         // (?<=exp)
  否定前瞻,     // (?<!exp)
  后瞻,         // (?=exp)
  否定后瞻,     // (?!exp)
  非捕获表达式, // (?:exp)
  注释,         // (?#exp)
  等待重复1,    // exp 等* +
  等待重复2,    // exp 等？
  括号问号,     //（?
};

struct regex_string_value {
  std::string data;
  std::string name;
  regex_string_value(const std::string &data, const std::string &name)
      : data(data), name(name) {}
};

typedef std::list<regex_string_value> regex_string;

regex_string generate_regex_string(const char *regex_ptr, std::size_t size);
