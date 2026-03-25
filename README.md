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
| LilyGo T-Dongle S3 | Yes |
| Adafruit QT Py ESP32-S3 | No |

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

## Adding a New Remote

1. Enable debug mode in `sdkconfig.defaults`:
   ```
   CONFIG_DEBUG_MODE=y
   ```
2. Build, flash, and connect to serial monitor (`115200` baud)
3. Pair the new remote — the HID Report Map descriptor will be dumped to serial
4. Copy the descriptor to a new file in `main/remotes/your_model.h`
5. Add the model choice in `main/Kconfig.projbuild`
6. Add the `#elif` in `main/amazon_remote_desc.h`
7. Disable debug mode and rebuild

## Credits

This project is based on [esp-usb-ble-hid](https://github.com/finger563/esp-usb-ble-hid) by [William Emfinger (@finger563)](https://github.com/finger563), originally designed as a BLE gamepad to USB bridge for the Nintendo Switch. This fork adapts it for Amazon Fire TV Remotes.

## License

See the original project for license details.
