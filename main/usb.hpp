#pragma once

#include <cstdint>
#include <vector>

#include "logger.hpp"

extern "C" {
#include <class/hid/hid_device.h>
#include <tinyusb.h>
#include <tusb.h>
}

#include "tinyusb_default_config.h"

void start_usb_hid();
bool send_hid_report(uint8_t report_id, const std::vector<uint8_t> &report);
void stop_usb_hid();
