#include "../remote.h"

extern "C" {
#include <class/hid/hid_device.h>
#include <tusb.h>
}

/************* process_ble_report — Fire TV Remote pass-through ****************/

void process_ble_report(int report_id, uint8_t *data, size_t length) {
  // Direct pass-through: forward BLE HID reports to USB unchanged.
  // The Fire TV Remote descriptor is already clean (no vendor collections),
  // so the OS handles all usages natively.
  tud_hid_report(report_id, data, length);
}
