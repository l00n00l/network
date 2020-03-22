#pragma once
#include <cstddef>
#include <list>
#include <string>
#include <unordered_map>
enum regex_op {
  ���Ͽ�ʼ�� = 0,          // [     ===>  [a-zA-Z0-9]
  ������ʼ�� = 1,          // [^    ===>  [^a-zA-Z0-9]
  ���Ϸ�Χ�� = 2,          // -     ===>  [a-zA-Z0-9] or [^a-zA-Z0-9]
  ���Ϲرշ� = 3,          // ]     ===>  [a-zA-Z0-9] or [^a-zA-Z0-9]
  �ظ���ʼ�� = 4,          // {     ===>  {n1, n2} or {n}
  �ظ��ָ��� = 5,          // ,     ===>  {n1, n2}
  �ظ��رշ� = 6,          // }     ===>  {n1, n2} or {n}
  ���ʽ��ʼ�� = 7,        // (     ===>  (exp|exp)
  ���ʽ�л��� = 8,        // |     ===>  (exp|exp)
  ���ƿ�ʼ�� = 9,          // (?<   ===>  (?<name>exp)
  ���ƹرշ� = 10,         // >     ===>  (?<name>exp)
  ǰհ��ʼ�� = 11,         // (?<=  ===>  (?<=exp)
  ��ǰհ��ʼ�� = 12,     // (?<!  ===>  (?<!exp)
  ��հ��ʼ�� = 13,         // (?=   ===>  (?=exp)
  �񶨺�հ��ʼ�� = 14,     // (?!   ===>  (?!exp)
  �ǲ�����ʽ��ʼ�� = 15, // (?:   ===>  (?:exp)
  ���ʽ�رշ� = 16,       // ��    ===>   )
  �Ǻ� = 17,               // *     ===>   exp*
  �Ӻ� = 18,               // +     ===>   exp+
  �Ǻ��ʺ� = 19,           // *?    ===>   exp*?
  �Ӻ��ʺ� = 20,           // +?    ===>   exp+?
  ת��� = 21,             // \     ===>   \{
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