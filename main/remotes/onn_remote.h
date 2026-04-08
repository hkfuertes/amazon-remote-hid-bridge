#pragma once
#include <cstdint>

// ONN Remote — USB HID descriptor
//
// Consumer Control (Report ID 1): pass-through for Mute, Search/Mic
// Keyboard (Report ID 2): remapped buttons (D-pad, navigation, function keys)
//                          + Vol+/Vol- forwarded from BLE keyboard report
//
// See BUTTON_MAP.md for the full mapping table.

static const uint8_t remote_hid_desc[] = {
    // ========================
    // Consumer Control (Report ID 1)
    // ========================
    // 1x 16-bit usage code — only Mute (0x00E2) and Search (0x0221) pass through
    0x05, 0x0C,       // Usage Page (Consumer)
    0x09, 0x01,       // Usage (Consumer Control)
    0xA1, 0x01,       // Collection (Application)
    0x85, 0x01,       //   Report ID (1)
    0x95, 0x01,       //   Report Count (1)
    0x75, 0x10,       //   Report Size (16)
    0x15, 0x00,       //   Logical Minimum (0)
    0x26, 0xFF, 0x03, //   Logical Maximum (1023)
    0x19, 0x00,       //   Usage Minimum (0)
    0x2A, 0xFF, 0x03, //   Usage Maximum (1023)
    0x81, 0x00,       //   Input (Data, Array, Absolute)
    0xC0,             // End Collection

    // ========================
    // Keyboard (Report ID 2)
    // ========================
    // Standard keyboard: modifier(1) + reserved(1) + keycodes(6) = 8 bytes
    // Used for remapped buttons and Vol+/Vol- pass-through
    0x05, 0x01,       // Usage Page (Generic Desktop)
    0x09, 0x06,       // Usage (Keyboard)
    0xA1, 0x01,       // Collection (Application)
    0x85, 0x02,       //   Report ID (2)

    // Modifier byte (8 bits)
    0x05, 0x07,       //   Usage Page (Keyboard/Keypad)
    0x19, 0xE0,       //   Usage Minimum (Left Control)
    0x29, 0xE7,       //   Usage Maximum (Right GUI)
    0x15, 0x00,       //   Logical Minimum (0)
    0x25, 0x01,       //   Logical Maximum (1)
    0x75, 0x01,       //   Report Size (1)
    0x95, 0x08,       //   Report Count (8)
    0x81, 0x02,       //   Input (Data, Variable, Absolute)

    // Reserved byte
    0x95, 0x01,       //   Report Count (1)
    0x75, 0x08,       //   Report Size (8)
    0x81, 0x01,       //   Input (Constant)

    // LED output (3 LEDs + 5 padding) — required by some OS for keyboard class
    0x95, 0x03,       //   Report Count (3)
    0x75, 0x01,       //   Report Size (1)
    0x05, 0x08,       //   Usage Page (LEDs)
    0x19, 0x01,       //   Usage Minimum (Num Lock)
    0x29, 0x03,       //   Usage Maximum (Scroll Lock)
    0x91, 0x02,       //   Output (Data, Variable, Absolute)
    0x95, 0x05,       //   Report Count (5)
    0x75, 0x01,       //   Report Size (1)
    0x91, 0x01,       //   Output (Constant)

    // 6 keycodes
    0x95, 0x06,       //   Report Count (6)
    0x75, 0x08,       //   Report Size (8)
    0x15, 0x00,       //   Logical Minimum (0)
    0x26, 0xFF, 0x00, //   Logical Maximum (255)
    0x05, 0x07,       //   Usage Page (Keyboard/Keypad)
    0x19, 0x00,       //   Usage Minimum (0)
    0x2A, 0xFF, 0x00, //   Usage Maximum (255)
    0x81, 0x00,       //   Input (Data, Array, Absolute)

    0xC0,             // End Collection
};
static const uint16_t remote_hid_desc_len = sizeof(remote_hid_desc);
