#pragma once
#include "utils/conf.h"

conf g_config;

#define g_control_port g_config.get_int32("control_port", 12345)
