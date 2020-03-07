#pragma once
#include "rapidjson/document.h"
#include "rapidjson/istreamwrapper.h"
#include "rapidjson/ostreamwrapper.h"
#include "rapidjson/writer.h"
#include <fstream>
using namespace rapidjson;
using namespace std;

Document json_load_fs(const char *filename) {
  ifstream ifs(filename);
  IStreamWrapper isw(ifs);
  Document d;
  d.ParseStream(isw);
  ifs.close();
  return d;
}

Document json_load(const char *data_ptr, std::size_t data_size) {
  Document d;
  d.Parse(data_ptr, data_size);
  return d;
}

void json_write_fs(Document &d, const char *filename) {
  ofstream ofs(filename);
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
