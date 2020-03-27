#pragma once
#include "rapidjson/document.h"
#include "rapidjson/error/en.h"
#include "rapidjson/istreamwrapper.h"
#include "rapidjson/ostreamwrapper.h"
#include "rapidjson/writer.h"
#include <fstream>

rapidjson::Document json_load_fs(const char *filename);

rapidjson::Document json_load(const char *data_ptr, std::size_t data_size);

void json_write_fs(rapidjson::Document &d, const char *filename);

std::string json_to_string(rapidjson::Document &d);

std::string value_to_string(rapidjson::Value &v);
