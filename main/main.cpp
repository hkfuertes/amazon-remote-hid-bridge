#include <chrono>
#include <thread>
#include <cstdio>
#include <cstring>

#include "logger.hpp"
#include "task.hpp"
#include "high_resolution_timer.hpp"

#include "ble.hpp"
#include "bsp.hpp"
#include "usb.hpp"
#include "remote.h"

using namespace std::chrono_literals;

/************* App Configuration ****************/

static int battery_level_percent = 100;
static std::string serial_number = "";

/********* BLE callbacks ***************/

/** Notification / Indication receiving handler callback - BLE to USB HID bridge */
void notifyCB(NimBLERemoteCharacteristic *pRemoteCharacteristic, uint8_t *pData, size_t length,
              bool isNotify) {
  // Battery level characteristic
  if (pRemoteCharacteristic->getUUID().equals(
          NimBLEUUID(espp::BatteryService::BATTERY_LEVEL_CHAR_UUID))) {
    battery_level_percent = pData[0];
    return;
  }

  // Get the report ID from the Report Reference descriptor mapping
  int report_id = get_report_id_for_characteristic(pRemoteCharacteristic);
  if (report_id < 0 || length == 0) return;

#if CONFIG_DEBUG_MODE
  bool is_release_dbg = true;
  for (size_t i = 0; i < length; i++) {
    if (pData[i] != 0) { is_release_dbg = false; break; }
  }
  printf("[BTN] report=%d len=%u kind=%s data=", report_id, (unsigned)length,
         is_release_dbg ? "release" : "press");
  for (size_t i = 0; i < length; i++) {
    printf("%02X", pData[i]);
    if (i + 1 < length) printf(" ");
  }
  printf("\n");
#endif

  // Delegate to remote-specific processing (remap or pass-through)
  process_ble_report(report_id, pData, length);

  // Toggle LED on press events
  {
    bool is_release = true;
    for (size_t i = 0; i < length; i++) {
      if (pData[i] != 0) { is_release = false; break; }
    }
    if (!is_release) {
      static auto &bsp = Bsp::get();
      static bool led_on = false;
      static auto on_color = espp::Rgb(0.0f, 0.0f, 1.0f);
      static auto off_color = espp::Rgb(0.0f, 0.0f, 0.0f);
      bsp.led(led_on ? on_color : off_color);
      led_on = !led_on;
    }
  }
}

extern "C" void app_main(void) {
  espp::Logger logger({.tag = "ESP USB BLE HID", .level = espp::Logger::Verbosity::DEBUG});

  logger.info("Bootup");

  // MARK: BSP initialization
  auto &bsp = Bsp::get();

  // MARK: LED initialization
  bsp.initialize_led();
  bsp.led(espp::Rgb(0.0f, 0.0f, 0.0f));

  // MARK: BLE pairing timer (for use with button)
  espp::HighResolutionTimer ble_pairing_timer{
      {.name = "Pairing Timer", .callback = [&]() { start_ble_pairing_thread(notifyCB); }}};

  // MARK: Pairing button initialization
  logger.info("Initializing the button");
  auto on_button_pressed = [&](const auto &event) {
    if (event.active) {
      ble_pairing_timer.oneshot(3'000'000); // 3 seconds
    } else {
      ble_pairing_timer.stop();
    }
  };
  bsp.initialize_button(on_button_pressed);

  // MARK: USB initialization
#if !CONFIG_DEBUG_MODE
  logger.info("USB initialization");
  start_usb_hid();
#else
  logger.info("DEBUG MODE: USB HID disabled, using USB for serial console");
#endif

  // MARK: BLE initialization
  logger.info("BLE initialization");
  std::string device_name = "AmazonRemote";
  init_ble(device_name);

  logger.info("Scanning for peripherals");
  start_ble_reconnection_thread(notifyCB);

  // Main loop
  while (true) {
    std::this_thread::sleep_for(1s);

    if (is_ble_subscribed()) {
      if (serial_number.empty()) {
        serial_number = get_connected_client_serial_number();
      }
      continue;
    }
    serial_number = "";
  }
}
