#include "network/server_mgr.h"
#include "network/session_mgr.h"
#include "network/tcp_proto.h"
#include <list>
#include <thread>

int main() {
  io_context ioc;
  boost::asio::signal_set signals(ioc, SIGINT, SIGTERM);
  signals.async_wait([&ioc](auto, auto) { ioc.stop(); });
  g_session_mgr = std::make_shared<session_mgr>(ioc);
  g_server_mgr = std::make_shared<server_mgr>(ioc);
  regist_contol_handler(
      [](char *data_ptr, std::size_t size, uint64 session_id) {
        std::cout << session_id << " ";
        std::cout.write(data_ptr, size);
        std::cout << std::endl;
      });
  g_server_mgr->create_server(tcp::endpoint(tcp::v4(), 1234),
                              std::string("control"));
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