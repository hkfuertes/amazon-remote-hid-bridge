#pragma once
#include <cstdint>

// Amazon Fire TV Remote - Model: R-NZ 201-180360
// Simplified descriptor: only keyboard (report 1) + consumer (report 2)
// Strips vendor-specific reports (0xF0-0xF3) that can confuse some OS drivers
static const uint8_t amazon_remote_hid_desc[] = {
    // Keyboard (Report ID 1): 3 keycodes
    0x05, 0x01,       // Usage Page (Generic Desktop)
    0x09, 0x06,       // Usage (Keyboard)
    0xA1, 0x01,       // Collection (Application)
    0x85, 0x01,       //   Report ID (1)
    0x05, 0x07,       //   Usage Page (Keyboard/Keypad)
    0x95, 0x03,       //   Report Count (3)
    0x75, 0x08,       //   Report Size (8)
    0x15, 0x00,       //   Logical Minimum (0)
    0x25, 0xFF,       //   Logical Maximum (255)
    0x19, 0x00,       //   Usage Minimum (0)
    0x29, 0xFF,       //   Usage Maximum (255)
    0x81, 0x00,       //   Input (Data, Array, Absolute)
    0xC0,             // End Collection

    // Consumer Control (Report ID 2): 2x 16-bit usage codes
    0x05, 0x0C,       // Usage Page (Consumer)
    0x09, 0x01,       // Usage (Consumer Control)
    0xA1, 0x01,       // Collection (Application)
    0x85, 0x02,       //   Report ID (2)
    0x95, 0x02,       //   Report Count (2)
    0x75, 0x10,       //   Report Size (16)
    0x15, 0x00,       //   Logical Minimum (0)
    0x26, 0x9C, 0x02, //   Logical Maximum (668)
    0x19, 0x00,       //   Usage Minimum (0)
    0x2A, 0x9C, 0x02, //   Usage Maximum (668)
    0x81, 0x00,       //   Input (Data, Array, Absolute)
    0xC0,             // End Collection
};
static const uint16_t amazon_remote_hid_desc_len = sizeof(amazon_remote_hid_desc);
