# atv-hid-bridge

ESP32-S3 firmware that acts as a BLE-to-USB HID bridge for Android TV remotes. It connects to the remote via Bluetooth Low Energy, receives HID reports, and forwards them over USB — making the remote work as a standard USB HID device on any computer (Windows, macOS, Linux).

## Supported Remotes

| Model | Mode | Status |
|-------|------|--------|
| R-NZ 201-180360 (Amazon Fire TV) | Pass-through | Tested |
| ONN Remote | Remap | Tested |

To add a new remote model, see [Adding a New Remote](#adding-a-new-remote).

## Supported Hardware

| Board | Display |
|-------|---------|
| Waveshare ESP32-S3-Zero | No |

## Quick Start

### Pre-built Binary

1. Download `atv-hid-bridge.bin` from the [releases page](https://github.com/hkfuertes/amazon-remote-hid-bridge/releases)
2. Connect the ESP32-S3 via USB
3. Put it in bootloader mode (hold BOOT, press RESET, release BOOT)
4. Flash at offset `0x0` using [WebSerial](https://web.esphome.io) or `esptool.py`:
   ```sh
   esptool.py --chip esp32s3 write_flash 0x0 atv-hid-bridge.bin
   ```
5. Reset the board and pair your remote (see [Usage](#usage))

### Build with Docker

No ESP-IDF installation needed:

```sh
./build.sh
```

This produces `atv-hid-bridge.bin` ready to flash.

### Build with ESP-IDF

Requires ESP-IDF v5.5.1+:

```sh
idf.py set-target esp32s3
idf.py build
idf.py -p PORT flash
```

## Configuration

Run `idf.py menuconfig` to configure:

- **Hardware Configuration** > **Target Hardware** — select your board
- **BLE Remote Configuration** > **Remote model** — select your remote model
- **Debug Configuration** > **Debug mode** — enable serial console (disables USB HID) for capturing new remote descriptors

## Usage

### Pairing

On first boot (no bonded devices), the bridge enters pairing mode automatically (LED breathes blue). Put your remote in pairing mode — they will connect and bond.

To pair a new remote later: **hold the BOOT button for 3 seconds** until the LED starts breathing faster.

The bridge can store up to 5 bonded devices.

### Reconnection

After pairing, the bridge automatically reconnects to bonded remotes on boot. The LED breathes blue while scanning and turns off once connected. Remotes using BLE Privacy (RPA) are handled automatically via the bond store.

### Connected

Once connected, button presses on the remote are forwarded over USB as standard HID reports. The LED toggles blue on each button press.

### Button Maps

#### R-NZ 201-180360 (Fire TV Remote) — Pass-through

All HID reports are forwarded directly without remapping.

| Remote Button | HID Report |
|---------------|------------|
| Up / Down / Left / Right | Keyboard arrow keys |
| Select (OK) | Keyboard Enter |
| Volume Up / Down | Consumer Volume Up/Down |
| Mute | Consumer Mute |
| Play/Pause | Consumer Play/Pause |
| Fast Forward / Rewind | Consumer FF/Rewind |
| Home | Consumer Home (0x0223) |
| Menu | Consumer Menu (0x0040) |

#### ONN Remote — Remapped

The ONN remote requires remapping because its raw BLE descriptor contains vendor collections that cause the OS to misclassify the device. See [BUTTON_MAP.md](BUTTON_MAP.md) for the full mapping table.

| Remote Button | USB Output |
|---------------|------------|
| D-pad Up/Down/Left/Right | Keyboard Arrow keys |
| OK/Enter | Keyboard Enter |
| Back | Keyboard Escape |
| Home | Keyboard B |
| Volume Up/Down | Consumer Volume Up/Down |
| Mute | Consumer Mute |
| Search/Mic | Consumer Search (0x0221) |
| Channel +/- | Keyboard Page Up/Down |
| Power | Keyboard F13 |
| TV Source | Keyboard F14 |
| User | Keyboard F15 |
| Settings | Keyboard F16 |
| TV Guide | Keyboard F5 |
| App 1-4 | Keyboard F18-F21 |

## Debug Mode (Serial Console)

The ESP32-S3-Zero only has one USB port. In normal mode it is used for USB HID, so there is no serial output. **Debug mode** disables USB HID and uses that port as a serial console instead, allowing you to see BLE scan results, connection events, HID descriptors, and button presses.

To enable debug mode, set in `sdkconfig.defaults`:

```
CONFIG_DEBUG_MODE=y
CONFIG_ESP_CONSOLE_USB_SERIAL_JTAG=y
```

Then build and flash. Connect to the serial console at **115200 baud**:

```sh
# macOS
screen /dev/tty.usbmodem101 115200

# Linux
screen /dev/ttyACM0 115200

# Windows (find COM port in Device Manager)
# Use PuTTY or similar at 115200 baud
```

To return to normal (USB HID) mode, remove or comment out those two lines and rebuild.

## Adding a New Remote

1. Enable debug mode (see above)
2. Build and flash
3. Put the new remote in pairing mode — hold the **BOOT** button on the ESP32 for 3 seconds to enter pairing
4. The serial console will show:
   - `[BLE] Connected` — connection established
   - `[BLE] HID Service has N characteristics` — with Report Reference IDs
   - `========== HID REPORT MAP DUMP ==========` — the full HID descriptor
   - `[BTN] report=... data=...` — button presses with raw hex data
5. Copy the descriptor dump to a new file in `main/remotes/your_model.h`
6. If needed, create a `main/remotes/your_model.cpp` implementing `process_ble_report()` (see `main/remote.h` for the interface)
7. Add the model as a new `config` entry in `main/Kconfig.projbuild` under `choice REMOTE_MODEL`
8. Add the corresponding `#elif` in `main/remote_desc.h`
9. Disable debug mode and rebuild

## Credits

This project is based on [esp-usb-ble-hid](https://github.com/finger563/esp-usb-ble-hid) by [William Emfinger (@finger563)](https://github.com/finger563), originally designed as a BLE gamepad to USB bridge for the Nintendo Switch. This fork adapts it for Android TV remotes.

## License

See the original project for license details.
