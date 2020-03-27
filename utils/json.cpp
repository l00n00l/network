#include "json.h"
#include "log.h"
#include <boost/locale.hpp>
#include <sstream>

using namespace rapidjson;

Document json_load_fs(const char *filename) {
  std::ifstream ifs(filename);
  IStreamWrapper isw(ifs);
  Document d;
  ParseResult ok = d.ParseStream(isw);
  if (!ok) {
    std::stringstream ss;
    ss << u8"在函数json_load_fs中加载json出现问题，问题码："
       << GetParseError_En(ok.Code()) << ok.Offset() << "\n";
    log_error(ss.str());
  }
  ifs.close();
  return d;
}

Document json_load(const char *data_ptr, std::size_t data_size) {
  Document d;
  ParseResult ok = d.Parse(data_ptr, data_size);
  if (!ok) {
    std::stringstream ss;
    ss << u8"在函数json_load中加载json出现问题，问题码："
       << GetParseError_En(ok.Code()) << ok.Offset() << "\n";
    log_error(ss.str());
  }
  return d;
}

void json_write_fs(Document &d, const char *filename) {
  std::ofstream ofs(filename);
  OStreamWrapper osw(ofs);
  Writer<OStreamWrapper> writer(osw);
  d.Accept(writer);
  ofs.close();
}

std::string json_to_string(Document &d) {
  StringBuffer buffer;
  Writer<StringBuffer> writer(buffer);
  d.Accept(writer);
  return std::string(buffer.GetString(), buffer.GetSize());
}

std::string value_to_string(Value &v) {
  return std::string(v.GetString(), v.GetStringLength());
}
