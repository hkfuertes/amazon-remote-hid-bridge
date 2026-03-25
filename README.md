# Amazon Remote HID Bridge

ESP32-S3 firmware that acts as a BLE-to-USB HID bridge for Amazon Fire TV Remotes. It connects to the remote via Bluetooth Low Energy, receives HID reports, and forwards them over USB — making the remote work as a standard USB HID device on any computer (Windows, macOS, Linux).

## Supported Remotes

| Model | Status |
|-------|--------|
| R-NZ 201-180360 | Tested |

To add a new remote model, see [Adding a New Remote](#adding-a-new-remote).

## Supported Hardware

| Board | Display |
|-------|---------|
| Waveshare ESP32-S3-Zero | No |

## Quick Start

### Pre-built Binary

1. Download `amazon-remote-hid-bridge.bin` from the [releases page](https://github.com/hkfuertes/amazon-remote-hid-bridge/releases)
2. Connect the ESP32-S3 via USB
3. Put it in bootloader mode (hold BOOT, press RESET, release BOOT)
4. Flash at offset `0x0` using [WebSerial](https://web.esphome.io) or `esptool.py`:
   ```sh
   esptool.py --chip esp32s3 write_flash 0x0 amazon-remote-hid-bridge.bin
   ```
5. Reset the board and pair your remote (see [Usage](#usage))

### Build with Docker

No ESP-IDF installation needed:

```sh
./build.sh
```

This produces `amazon-remote-hid-bridge.bin` ready to flash.

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

On first boot (no bonded devices), the bridge enters pairing mode automatically (LED breathes blue). Put your Amazon remote in pairing mode — they will connect and bond.

To pair a new remote later: **hold the BOOT button for 3 seconds** until the LED starts breathing faster.

The bridge can store up to 5 bonded devices.

### Reconnection

After pairing, the bridge automatically reconnects to bonded remotes on boot. The LED breathes blue while scanning and turns off once connected.

### Connected

Once connected, button presses on the remote are forwarded over USB as standard HID reports. The LED toggles blue on each button press.

### Button Map

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
   - `[BUTTON] X pressed` — button presses with identified names, or raw hex for unknown buttons
5. Copy the descriptor dump to a new file in `main/remotes/your_model.h`
6. Add the model as a new `config` entry in `main/Kconfig.projbuild` under `choice REMOTE_MODEL`
7. Add the corresponding `#elif` in `main/amazon_remote_desc.h`
8. Disable debug mode and rebuild

## Credits

This project is based on [esp-usb-ble-hid](https://github.com/finger563/esp-usb-ble-hid) by [William Emfinger (@finger563)](https://github.com/finger563), originally designed as a BLE gamepad to USB bridge for the Nintendo Switch. This fork adapts it for Amazon Fire TV Remotes.

## License

See the original project for license details.
