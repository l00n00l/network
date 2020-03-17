#pragma once
#include <cstddef>
#include <list>
#include <string>
enum regex_type {
  ��ͨ�ַ� = 1, // ��ͨ�ַ�
  ����,         // [char] // ��֧�ַ�Χ-
  �񶨼���,     // [^char]// ��֧�ַ�Χ-
  �ظ�����,     // {n1,n2}
  �ظ�����1,    // {n1}
  ����,         // (exp)
  ����,         // (exp|exp)
  ����,         //��?<name>exp)
  ���պ�����,   //��?<
  ǰհ,         // (?<=exp)
  ��ǰհ,     // (?<!exp)
  ��հ,         // (?=exp)
  �񶨺�հ,     // (?!exp)
  �ǲ�����ʽ, // (?:exp)
  ע��,         // (?#exp)
  �ȴ��ظ�1,    // exp ��* +
  �ȴ��ظ�2,    // exp �ȣ�
  �����ʺ�,     //��?
};

struct regex_string_value {
  std::string data;
  std::string name;
  regex_string_value(const std::string &data, const std::string &name)
      : data(data), name(name) {}
};

typedef std::list<regex_string_value> regex_string;

regex_string generate_regex_string(const char *regex_ptr, std::size_t size);
