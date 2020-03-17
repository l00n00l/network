#pragma once
#include "utils/conf.h"

conf g_config;

#define g_control_port g_config.get_int32("control_port", 12345)
#define g_proto_path g_config.get_str("proto_path", "E:/c++/vcpu/protos/")
