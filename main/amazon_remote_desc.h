#pragma once

// Select the HID descriptor based on the configured remote model
#if CONFIG_REMOTE_R_NZ_201_180360
#include "remotes/r_nz_201_180360.h"
#else
#error "No remote model selected. Configure in menuconfig -> BLE Remote Configuration."
#endif
