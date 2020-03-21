#include "utils/json.h"
#include "utils/regex_string.h"
#include "utils/utils.h"
#include <boost/any.hpp>
#include <iostream>
int main() {
  // regex r;
  // r.set_expression("(?<=(?:bbb))(?<hahaha>.*)");
  // auto s = std::string("sdfjkalsjbbb234234    fjksdljf");
  // boost::match_results<std::string::const_iterator> m;
  // if (regex_search(s, m, r)) {
  //}
  // std::string s;

  // r.set_expression("(\\w+$)");
  // if (boost::regex_search(s, m, r)) {
  //  std::cout << m[0].str() << std::endl;
  //  std::cout << m[1].str() << std::endl;
  //  std::cout << m.size() << std::endl;
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
  set_error_log_func([](const char *data_ptr, std::size_t size) {
    std::cerr << from_utf(u8"[´íÎó]", "gb2312")
              << from_utf(std::string(data_ptr, size), "gb2312") << std::endl;
  });
  set_info_log_func([](const char *data_ptr, std::size_t size) {
    std::cout << from_utf(u8"[ÐÅÏ¢]", "gb2312")
              << from_utf(std::string(data_ptr, size), "gb2312") << std::endl;
  });
  set_debug_log_func([](const char *data_ptr, std::size_t size) {
    std::cout << from_utf(u8"[µ÷ÊÔ]", "gb2312")
              << from_utf(std::string(data_ptr, size), "gb2312") << std::endl;
  });

  regstring s;
  s.parse_regex("(?<key_0>.*):{0,1}(?<value_0>.*)\r\n");
  s.set("key_0", "hahaha");
  s.set("value_0", "HTTP/1.1");
  std::cout << s.str();
  return 0;
}

//
// async_tcp_echo_server.cpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2019 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
//
//#include <boost/asio.hpp>
//#include <boost/regex.hpp>
//#include <cstdlib>
//#include <iostream>
//#include <memory>
//#include <utility>
// using boost::asio::ip::tcp;
//
// class session : public std::enable_shared_from_this<session> {
// public:
//  session(tcp::socket socket) : socket_(std::move(socket)) {}
//
//  void start() { do_read(); }
//
// private:
//  void do_read() {
//    auto self(shared_from_this());
//    boost::asio::async_read_until(
//        socket_, sb, boost::regex("\r\n"),
//        [this, self](boost::system::error_code ec, std::size_t length) {
//          auto s = std::string(boost::asio::buffers_begin(sb.data()),
//                               boost::asio::buffers_begin(sb.data()) +
//                               length);
//          boost::match_results<std::string::const_iterator> mrets;
//          auto ret = boost::regex_search(
//              s, mrets,
//              boost::regex(
//                  "(?<action>^\\w+)\\s{1}(?<url>\\S+)\\s{1}(?<version>\\S+)"));
//          if (ret) {
//            std::cout << mrets["action"] << std::endl;
//            std::cout << mrets["url"] << std::endl;
//            std::cout << mrets["version"] << std::endl;
//          }
//          sb.consume(length);
//        });
//  }
//
//  void do_write(std::size_t length) {
//    auto self(shared_from_this());
//    boost::asio::async_write(
//        socket_, boost::asio::buffer(data_, length),
//        [this, self](boost::system::error_code ec, std::size_t /*length*/) {
//          if (!ec) {
//            do_read();
//          }
//        });
//  }
//
//  tcp::socket socket_;
//  enum { max_length = 1024 };
//  char data_[max_length];
//  boost::asio::streambuf sb;
//};
//
// class server {
// public:
//  server(boost::asio::io_context &io_context, short port)
//      : acceptor_(io_context, tcp::endpoint(tcp::v4(), port)) {
//    do_accept();
//  }
//
// private:
//  void do_accept() {
//    acceptor_.async_accept(
//        [this](boost::system::error_code ec, tcp::socket socket) {
//          if (!ec) {
//            std::make_shared<session>(std::move(socket))->start();
//          }
//
//          do_accept();
//        });
//  }
//
//  tcp::acceptor acceptor_;
//};
//
// int main(int argc, char *argv[]) {
//  try {
//    boost::asio::io_context io_context;
//    server s(io_context, 1234);
//    io_context.run();
//  } catch (std::exception &e) {
//    std::cerr << "Exception: " << e.what() << "\n";
//  }
//
//  return 0;
//}