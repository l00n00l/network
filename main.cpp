#include "cfg.h"
#include "network/server_mgr.h"
#include "network/session_mgr.h"
#include "network/tcp_proto.h"
#include "utils/dicts.h"
#include <list>
#include <thread>
int main() {
  set_error_log_func([](const char *data_ptr, std::size_t size) {
    std::cerr << from_utf(u8"[错误]", "gb2312")
              << from_utf(std::string(data_ptr, size), "gb2312") << std::endl;
  });
  set_info_log_func([](const char *data_ptr, std::size_t size) {
    std::cout << from_utf(u8"[信息]", "gb2312")
              << from_utf(std::string(data_ptr, size), "gb2312") << std::endl;
  });
  set_debug_log_func([](const char *data_ptr, std::size_t size) {
    std::cout << from_utf(u8"[调试]", "gb2312")
              << from_utf(std::string(data_ptr, size), "gb2312") << std::endl;
  });

  // 初始化配置文件
  g_config.init("config.json");

  // 加载协议信息
  if (!load_protos(g_proto_path)) {
    return EXIT_FAILURE;
  };

  set_message_handler(
      [](uint64 session_id, uint64 data_id, const char *proto_name) {
        extern dicts g_net_dicts;
        dict_iterator itr(data_id, &g_net_dicts);
        for (auto key = itr(); !key.empty(); key = itr()) {
          lsdebug << key << ":" >> g_net_dicts.get_string(data_id, key);
        }
      });

  io_context ioc;
  boost::asio::signal_set signals(ioc, SIGINT, SIGTERM);
  signals.async_wait([&ioc](auto, auto) { ioc.stop(); });
  g_session_mgr = std::make_shared<session_mgr>(ioc);
  g_server_mgr = std::make_shared<server_mgr>(ioc);

  g_server_mgr->create_server(tcp::endpoint(tcp::v4(), g_control_port), "http");
  // g_session_mgr->create_session(std::string("control"),
  //                              std::string("localhost"),
  //                              std::string("12345"));
  auto core_count = std::thread::hardware_concurrency();
  std::list<std::thread> thread_list;
  for (size_t i = 1; i < core_count; i++) {
    thread_list.push_back(std::thread([&ioc] { ioc.run(); }));
  }

  ioc.run();

  for (auto &i : thread_list) {
    i.join();
  }
}