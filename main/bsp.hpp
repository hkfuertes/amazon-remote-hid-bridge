#pragma once

#if CONFIG_TARGET_HARDWARE_ESP32S3_ZERO
#include "esp32s3_zero.hpp"
using Bsp = espp::Esp32s3Zero;
#else
#error "No hardware target specified"
#endif
