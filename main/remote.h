#pragma once

#include <cstdint>
#include <cstddef>

// Common interface for remote-specific BLE report processing.
// Each remote model implements this function in its own .cpp file.
// The build system links the correct implementation based on Kconfig.
void process_ble_report(int report_id, uint8_t *data, size_t length);
