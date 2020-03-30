# 一个简单的网络库

本库可以根据自己的意愿定制协议

## 协议例子（http)

```json
{
  "server": [
    {
      "read_until": "\r\n",
      "complie_regex": "(?<action>\\w+)\\s(?<url>\\S+)\\s(?<version>\\S+)\r\n",
      "next_condition": ""
    },
    {
      "read_until": "\r\n",
      "complie_regex": "(?<key_0>.*?):\\s(?<value_0>.*)\r\n",
      "next_condition": "\r\n"
    },
    {
      "read_size": "Content-Length",
      "complie_regex": "<?<data>[\\s\\S]*)",
      "next_condition": ""
    }
  ],
  "client": [
    {
      "read_until": "\r\n",
      "complie_regex": "(?<version>\\w+)\\s(?<status_code>\\S+)\\s(?<status>\\S+)\r\n",
      "next_condition": ""
    },
    {
      "read_until": "\r\n",
      "complie_regex": "(?<key_0>.*?):\\s(?<value_0>.*)\r\n",
      "next_condition": "\r\n"
    },
    {
      "read_size": "Content-Length",
      "complie_regex": "<?<data>[\\s\\S]*)",
      "next_condition": ""
    }
  ]
}

```

## 协议解释

协议文件是一个json文件，里面有两个字段server和client，server表示服务器方的协议，client时客户端的协议。没个协议都是一个列表，列表里面是读取规则。读取数据的时候从第一项开始到最后一项截至。读完后便出发message_handler把生成的数据变量传递给处理函数。


## 字段解释

* read_some:随机读取一些tcp数据包，读取多少不确定。
* read_until:值为正则表达式,读取数据时会匹配正则表达式。如果匹配成功就停止读取
* read_size:值可以是(string字符串)也可(uint整型)。读取固定大小的数据。当其值是字符串时表示一个已经读取的值的变量名称。当值为uint时表示读取数据的大小。
* complie_regex：将数据根据正则表达式分割并生成变量。```(?<var_name>exp)```其中var_name就是生成的变量名称，变量的值就是exp所匹配到的字符串。但当出现```(?<key_0>exp) 或者 (?<value_0>exp)```时key_0所匹配的字符串就会被当做变量名，value_0所匹配的字符串被当做与key_0对应变量的值。key_0对应value_0，key_1对应value_1,以此类推。
* next_condition：是否读取下一项的数据。值为正则表达式或这空字符串。空字符串表示直接读取下一项。如果时正则表达式就将本次读取的所有数据与之匹配，如果匹配成功则进入下一项的读取，否则继续按当前项读取数据。

**注意**
read_until,read_some,read_size三者只能出现一个。read_some缺省的,也就是说如果没有read_until和read_size就是read_some。

## 例子

```c++
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

  g_server_mgr->create_server(tcp::endpoint(tcp::v4(), g_control_port),
                              "control");

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
```
