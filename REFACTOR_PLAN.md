# atv-hid-bridge — Refactor Plan

## Completed

- [x] Rename project in `build.sh`, `Dockerfile`, `package_main.yml` → `atv-hid-bridge`
- [x] Rename `main/amazon_remote_desc.h` → `main/remote_desc.h`
- [x] Rename variable `amazon_remote_hid_desc` → `remote_hid_desc` in all files
  - `main/remotes/onn_remote.h`
  - `main/remotes/r_nz_201_180360.h`
  - `main/usb.cpp` (include + references)
  - `main/ble.cpp` (debug prints)
- [x] Extract ONN remap logic from `main.cpp` → `main/remotes/onn_remote.cpp`
- [x] Create `main/remote.h` — common interface (`process_ble_report`)
- [x] Create `main/remotes/r_nz_201_180360.cpp` — pass-through
- [x] Update `main/CMakeLists.txt` — add `remotes/` to `SRC_DIRS`
- [x] Clean up `main/usb.cpp` — generic USB strings, removed dead `send_hid_report()`, fixed log message
- [x] Simplify `main.cpp` — removed all `#if CONFIG_REMOTE_ONN` blocks, single `process_ble_report()` call
- [x] Update `README.md` — new name, ONN remote documented, stale references fixed
- [x] Build and verify — `./build.sh` succeeds, `atv-hid-bridge.bin` generated

## Architecture after refactor

```
main/
  main.cpp          — BLE notify callback → process_ble_report()
  remote.h          — Common interface (process_ble_report)
  remote_desc.h     — Kconfig dispatcher for USB HID descriptors
  ble.cpp / ble.hpp — BLE GAP/GATT logic
  usb.cpp / usb.hpp — USB HID setup (generic strings)
  bsp.hpp           — BSP dispatcher
  remotes/
    onn_remote.h              — USB HID descriptor (ONN)
    onn_remote.cpp            — ONN remap table + process_ble_report()
    r_nz_201_180360.h         — USB HID descriptor (Fire TV)
    r_nz_201_180360.cpp       — Pass-through process_ble_report()
```

## Notes

- Remapping only applies to ONN (`CONFIG_REMOTE_ONN`). Fire TV Remote uses raw pass-through.
- Only one remote `.cpp` is compiled per build (selected by Kconfig `REMOTE_MODEL` choice).
- `CMakeLists.txt` root project name stays `esp-usb-ble-hid` (used by `idf.py` to name the `.bin` file, referenced in Dockerfile).
- `main.cpp` reduced from 263 to 127 lines after extracting remote-specific logic.
