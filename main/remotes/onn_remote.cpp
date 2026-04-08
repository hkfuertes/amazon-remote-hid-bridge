#include <chrono>
#include <thread>
#include <cstdio>

#include "../remote.h"

extern "C" {
#include <class/hid/hid_device.h>
#include <tusb.h>
}

using namespace std::chrono_literals;

/************* ONN Remote — unified remap table ****************/

// USB report IDs (our descriptor)
static constexpr uint8_t USB_CONSUMER = 1;  // USB Report ID 1 — Consumer Control (2 bytes)
static constexpr uint8_t USB_KEYBOARD = 2;  // USB Report ID 2 — Keyboard (8 bytes)

// Unified remap entry: BLE input → USB output
// input_key encodes {ble_report_id, code} into a single uint32_t
struct OnnRemap {
  uint32_t input_key;    // (ble_report_id << 16) | input_code
  uint8_t  usb_report;   // USB_CONSUMER or USB_KEYBOARD
  uint16_t output_code;  // Consumer usage (16-bit) or keyboard keycode (8-bit)
};

static constexpr uint32_t make_key(uint8_t report_id, uint16_t code) {
  return (static_cast<uint32_t>(report_id) << 16) | code;
}

// BLE Report ID 1 = Consumer, BLE Report ID 11 (0x0B) = Keyboard
static constexpr OnnRemap onn_remap[] = {
  // --- Consumer pass-through (BLE consumer → USB consumer) ---
  { make_key(1, 0x00E2), USB_CONSUMER, 0x00E2 }, // Mute
  { make_key(1, 0x0221), USB_CONSUMER, 0x0221 }, // Search/Mic

  // --- Consumer → Keyboard remap ---
  { make_key(1, 0x0042), USB_KEYBOARD, 0x52 },   // D-pad Up    → Arrow Up
  { make_key(1, 0x0043), USB_KEYBOARD, 0x51 },   // D-pad Down  → Arrow Down
  { make_key(1, 0x0044), USB_KEYBOARD, 0x50 },   // D-pad Left  → Arrow Left
  { make_key(1, 0x0045), USB_KEYBOARD, 0x4F },   // D-pad Right → Arrow Right
  { make_key(1, 0x0041), USB_KEYBOARD, 0x28 },   // OK/Enter    → Enter
  { make_key(1, 0x0224), USB_KEYBOARD, 0x29 },   // Back        → Escape
  { make_key(1, 0x0223), USB_KEYBOARD, 0x05 },   // Home        → B
  { make_key(1, 0x009C), USB_KEYBOARD, 0x4B },   // Channel +   → Page Up
  { make_key(1, 0x009D), USB_KEYBOARD, 0x4E },   // Channel -   → Page Down
  { make_key(1, 0x0030), USB_KEYBOARD, 0x68 },   // Power       → F13
  { make_key(1, 0x019C), USB_KEYBOARD, 0x69 },   // TV Source   → F14
  { make_key(1, 0x01BB), USB_KEYBOARD, 0x6A },   // User        → F15
  { make_key(1, 0x009F), USB_KEYBOARD, 0x6B },   // Settings    → F16
  { make_key(1, 0x008D), USB_KEYBOARD, 0x3E },   // TV Guide    → F5
  { make_key(1, 0x0077), USB_KEYBOARD, 0x6D },   // App 1       → F18
  { make_key(1, 0x0078), USB_KEYBOARD, 0x6E },   // App 2       → F19
  { make_key(1, 0x0079), USB_KEYBOARD, 0x6F },   // App 3       → F20
  { make_key(1, 0x007A), USB_KEYBOARD, 0x70 },   // App 4       → F21

  // --- Keyboard → Consumer remap (Vol+/Vol-) ---
  { make_key(11, 0x0080), USB_CONSUMER, 0x00E9 }, // Vol+  → Consumer Volume Up
  { make_key(11, 0x0081), USB_CONSUMER, 0x00EA }, // Vol-  → Consumer Volume Down
};
static constexpr size_t ONN_REMAP_COUNT = sizeof(onn_remap) / sizeof(onn_remap[0]);

// Lookup: returns pointer to matching entry or nullptr
static const OnnRemap* onn_lookup(uint32_t key) {
  for (size_t i = 0; i < ONN_REMAP_COUNT; i++) {
    if (onn_remap[i].input_key == key) return &onn_remap[i];
  }
  return nullptr;
}

// Wait until TinyUSB endpoint is ready (max ~50 ms)
static void onn_wait_ready() {
  for (int i = 0; i < 50; i++) {
    if (tud_hid_ready()) return;
    std::this_thread::sleep_for(1ms);
  }
}

// Send press+release for any report type
static void onn_send(uint8_t usb_report, uint16_t code) {
  if (usb_report == USB_CONSUMER) {
    uint8_t buf[2] = { static_cast<uint8_t>(code & 0xFF), static_cast<uint8_t>(code >> 8) };
    onn_wait_ready();
    tud_hid_report(USB_CONSUMER, buf, 2);
    onn_wait_ready();
    buf[0] = buf[1] = 0;
    tud_hid_report(USB_CONSUMER, buf, 2);
  } else {
    uint8_t buf[8] = {0};
    buf[2] = static_cast<uint8_t>(code);
    onn_wait_ready();
    tud_hid_report(USB_KEYBOARD, buf, 8);
    onn_wait_ready();
    buf[2] = 0;
    tud_hid_report(USB_KEYBOARD, buf, 8);
  }
}

/************* process_ble_report — ONN implementation ****************/

void process_ble_report(int report_id, uint8_t *data, size_t length) {
  // Ignore release events (all zeros)
  bool is_release = true;
  for (size_t i = 0; i < length; i++) {
    if (data[i] != 0) { is_release = false; break; }
  }
  if (is_release) return;

  if (report_id == 1 && length >= 2) {
    // Consumer report: 16-bit usage in first 2 bytes (LE)
    uint32_t key = make_key(1, data[0] | (data[1] << 8));
    auto *r = onn_lookup(key);
    if (r) onn_send(r->usb_report, r->output_code);

  } else if (report_id == 11 && length >= 8) {
    // Keyboard report: scan keycodes in bytes 2..7
    for (int k = 2; k < 8; k++) {
      if (data[k] == 0) continue;
      uint32_t key = make_key(11, data[k]);
      auto *r = onn_lookup(key);
      if (r) onn_send(r->usb_report, r->output_code);
    }
  }
}
