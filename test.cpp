#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include <iostream>
int main() {

  rapidjson::Document d;
  d.Parse("{\"a\":123}");

  rapidjson::StringBuffer buffer;
  rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
  d.Accept(writer);

  std::cout << buffer.GetString() << buffer.GetSize() << std::endl;

  return 0;
}