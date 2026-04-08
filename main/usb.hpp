#pragma once

#include <cstdint>

#include "logger.hpp"

extern "C" {
#include <class/hid/hid_device.h>
#include <tinyusb.h>
#include <tusb.h>
}

#include "tinyusb_default_config.h"

void start_usb_hid();
void stop_usb_hid();
