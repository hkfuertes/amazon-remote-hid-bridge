#include <chrono>
#include <thread>

#include "logger.hpp"
#include "task.hpp"
#include "high_resolution_timer.hpp"

#include "ble.hpp"
#include "bsp.hpp"
#include "usb.hpp"

using namespace std::chrono_literals;

/************* App Configuration ****************/

#if HAS_DISPLAY
static std::shared_ptr<Gui> gui;
#endif
static int battery_level_percent = 100;
static std::string serial_number = "";

/********* BLE callbacks ***************/

/** Notification / Indication receiving handler callback - BLE to USB HID pass-through */
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

  // Forward to USB HID (both press and release events)
  if (!tud_hid_ready()) return;
  tud_hid_report(report_id, pData, length);

  // Toggle LED on press events (skip release/all-zeros)
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

extern "C" void app_main(void) {
  espp::Logger logger({.tag = "ESP USB BLE HID", .level = espp::Logger::Verbosity::DEBUG});

  logger.info("Bootup");

  // MARK: BSP initialization
  auto &bsp = Bsp::get();

  // MARK: LED initialization
  bsp.initialize_led();
  bsp.led(espp::Rgb(0.0f, 0.0f, 0.0f));

  // MARK: Display initialization
#if HAS_DISPLAY
  logger.info("Display initialization");
  // initialize the LCD
  if (!bsp.initialize_lcd()) {
    logger.error("Failed to initialize LCD!");
    return;
  }
  // set the pixel buffer to be a full screen buffer
  static constexpr size_t pixel_buffer_size = bsp.lcd_width() * bsp.lcd_height();
  // initialize the LVGL display for the T-Dongle-S3
  if (!bsp.initialize_display(pixel_buffer_size)) {
    logger.error("Failed to initialize display!");
    return;
  }

  // initialize the gui
  logger.info("Making GUI");
  gui = std::make_shared<Gui>(Gui::Config{.log_level = espp::Logger::Verbosity::INFO});
  gui->set_label_text("");
#else  // HAS_DISPLAY
  logger.info("No display");
#endif // HAS_DISPLAY

  // MARK: BLE pairing timer (for use with button)
  espp::HighResolutionTimer ble_pairing_timer{
      {.name = "Pairing Timer", .callback = [&]() { start_ble_pairing_thread(notifyCB); }}};

  // MARK: Pairing button initialization
  // initialize the button, which we'll use to cycle the rotation of the display
  logger.info("Initializing the button");
  auto on_button_pressed = [&](const auto &event) {
    if (event.active) {
      // start ble pairing timer
      ble_pairing_timer.oneshot(3'000'000); // 3 seconds
    } else {
      // cancel the ble pairing timer
      ble_pairing_timer.stop();
    }
  };
  bsp.initialize_button(on_button_pressed);

  // MARK: USB initialization
  logger.info("USB initialization");
  start_usb_hid();

  // MARK: BLE initialization
  logger.info("BLE initialization");
  std::string device_name = "AmazonRemote";
  init_ble(device_name);

  logger.info("Scanning for peripherals");
  start_ble_reconnection_thread(notifyCB);

  // Loop here until we find a device we want to connect to
  while (true) {
    // sleep for a bit
    std::this_thread::sleep_for(1s);

    // update the display if we have one
#if HAS_DISPLAY
    // show the usb icon if the USB is mounted
    gui->set_usb_connected(tud_mounted());
    // show the BLE icon if the BLE subsystem is subscribed (receiving data)
    gui->set_ble_connected(is_ble_subscribed());
#endif // HAS_DISPLAY

    // if we're subscribed, then don't do anything else
    if (is_ble_subscribed()) {
      // if we haven't gotten the serial number, then get that and save it
      if (serial_number.empty()) {
        serial_number = get_connected_client_serial_number();
#if HAS_DISPLAY
        gui->set_label_text(serial_number);
#endif // HAS_DISPLAY
      }
      continue;
    }
    // make sure to reset the connected device serial number
    serial_number = "";
#if HAS_DISPLAY
    gui->set_label_text(serial_number);
#endif // HAS_DISPLAY

  }
}
