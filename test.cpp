#include "utils/json.h"
#include "utils/regex_string.h"
#include "utils/utils.h"
#include <boost/python.hpp>
#include <Python.h>
#include <boost/any.hpp>
#include <iostream>
using namespace boost::python;

int main() {
  try {
    Py_Initialize();
    if (!Py_IsInitialized()) {
      return EXIT_FAILURE;
    }

    auto main_module = import("__main__");
    auto main_namespace = main_module.attr("__dict__");
    exec("print(__dict__)", main_namespace);
  } catch (error_already_set &) {
    if (PyErr_ExceptionMatches(PyExc_ZeroDivisionError)) {
      // handle ZeroDivisionError specially
    } else {
      // print all other errors to stderr
      PyErr_Print();
    }
  }

  // regex r;
  // r.set_expression("(?<=(?:bbb))(?<hahaha>.*)");
  // auto s = std::string("sdfjkalsjbbb234234    fjksdljf");
  // boost::match_results<std::string::const_iterator> m;
  // if (regex_search(s, m, r)) {
  //}
  // std::string s;

  // r.set_expression("(?<ttt>\\d+)");
  // if (boost::regex_search(s, m, r)) {
  //}
  // r.set_expression("\\w+\\s*\\w+");
  // if (boost::regex_match(s, r)) {
  //  std::cout << "yes" << std::endl;
  //} else {
  //  std::cout << "no" << std::endl;
  //}

  // set_error_log_func([](const char *data_ptr, std::size_t size) {
  //  std::cerr << from_utf(data_ptr, "gb2312");
  //});
  // auto json_str = "{\"a\":2}";
  // Document v = json_load(json_str, strlen(json_str));
  // v.AddMember("abc", 123, v.GetAllocator());
  // auto a = json_to_string(v);
  // std::cout << a << std::endl;
  // Value ks(kStringType);
  // ks.SetString("dsfdf", v.GetAllocator());

  // std::string a = "abc";
  // auto b = boost::lexical_cast<uint32>(a);
  // auto sss = value_to_string(v["a"]);
  // std::cout << sss << std::endl;
  // auto ret = typeid(int8) == typeid(int16);
  // std::cout << ret << std::endl;
  // boost::any v = uint16(2);
  // v = "fsdf";
  /* set_error_log_func([](const char *data_ptr, std::size_t size) {
     std::cerr << from_utf(u8"[´íÎó]", "gb2312")
               << from_utf(std::string(data_ptr, size), "gb2312") <<
   std::endl;
   });
   set_info_log_func([](const char *data_ptr, std::size_t size) {
     std::cout << from_utf(u8"[ÐÅÏ¢]", "gb2312")
               << from_utf(std::string(data_ptr, size), "gb2312") <<
   std::endl;
   });
   set_debug_log_func([](const char *data_ptr, std::size_t size) {
     std::cout << from_utf(u8"[µ÷ÊÔ]", "gb2312")
               << from_utf(std::string(data_ptr, size), "gb2312") <<
   std::endl;
   });

   regstring s;
   s.parse_regex("(?<key_0>.*):{0,1}(?<value_0>.*)\r\n");
   s.set("key_0", "hahaha");
   s.set("value_0", "HTTP/1.1");
   std::cout << s.str();*/
  return EXIT_SUCCESS;
}
