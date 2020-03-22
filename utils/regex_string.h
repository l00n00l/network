#pragma once
#include <cstddef>
#include <list>
#include <string>
#include <unordered_map>
enum regex_op {
  集合开始符 = 0,          // [     ===>  [a-zA-Z0-9]
  补集开始符 = 1,          // [^    ===>  [^a-zA-Z0-9]
  集合范围符 = 2,          // -     ===>  [a-zA-Z0-9] or [^a-zA-Z0-9]
  集合关闭符 = 3,          // ]     ===>  [a-zA-Z0-9] or [^a-zA-Z0-9]
  重复开始符 = 4,          // {     ===>  {n1, n2} or {n}
  重复分隔符 = 5,          // ,     ===>  {n1, n2}
  重复关闭符 = 6,          // }     ===>  {n1, n2} or {n}
  表达式开始符 = 7,        // (     ===>  (exp|exp)
  表达式切换符 = 8,        // |     ===>  (exp|exp)
  名称开始符 = 9,          // (?<   ===>  (?<name>exp)
  名称关闭符 = 10,         // >     ===>  (?<name>exp)
  前瞻开始符 = 11,         // (?<=  ===>  (?<=exp)
  否定前瞻开始符 = 12,     // (?<!  ===>  (?<!exp)
  后瞻开始符 = 13,         // (?=   ===>  (?=exp)
  否定后瞻开始符 = 14,     // (?!   ===>  (?!exp)
  非捕获表达式开始符 = 15, // (?:   ===>  (?:exp)
  表达式关闭符 = 16,       // ）    ===>   )
  星号 = 17,               // *     ===>   exp*
  加号 = 18,               // +     ===>   exp+
  星号问好 = 19,           // *?    ===>   exp*?
  加号问好 = 20,           // +?    ===>   exp+?
  转义符 = 21,             // \     ===>   \{
};

class regstring {
public:
  regstring();
  ~regstring();

  bool parse_regex(const char *regex_ptr, std::size_t size);
  bool parse_regex(const std::string &regex_str);
  bool set(const char *name, const char *data, std::size_t size);
  bool set(const char *name, const char *data);

  const char *c_str();
  std::size_t size();
  std::string str();

  std::list<std::string> names();

private:
  struct impl;
  impl *impl_ptr;
};