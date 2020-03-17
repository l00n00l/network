#pragma once
#include "rapidjson/document.h"
#include "rapidjson/error/en.h"
#include "rapidjson/istreamwrapper.h"
#include "rapidjson/ostreamwrapper.h"
#include "rapidjson/writer.h"
#include <fstream>
using namespace rapidjson;
using namespace std;

Document json_load_fs(const char *filename);

Document json_load(const char *data_ptr, std::size_t data_size);

void json_write_fs(Document &d, const char *filename);

std::string json_to_string(Document &d);

std::string value_to_string(Value &v);