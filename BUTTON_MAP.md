# ONN Remote — Button Map

The ONN remote's raw BLE HID descriptor contains vendor-specific Usage Pages
(0xFF81, 0xFF82) and a mouse collection that cause mainstream OSes to
misclassify the device as a vendor device, breaking Consumer Control reports.

Additionally, most Consumer Control usages sent by the ONN remote (Menu Up/Down,
AC Home, AC Back, etc.) are **ignored by macOS and Windows** even though they
arrive correctly over USB. Only Mute (0x00E2) and AC Search (0x0221) produce
an action. Therefore, buttons that don't work as Consumer pass-through are
remapped to keyboard keycodes.

**Solution**: A clean USB descriptor (Keyboard + Consumer only) is exposed over
USB, and BLE reports are remapped in firmware. This only applies when
`CONFIG_REMOTE_ONN=y`; other remotes (e.g. `r_nz_201_180360`) use direct
pass-through.

## USB Descriptor

| Report ID | Type | Size | Description |
|-----------|------|------|-------------|
| 1 | Consumer Control | 2 bytes | 16-bit usage code (LE) |
| 2 | Keyboard | 8 bytes | modifier(1) + reserved(1) + keycodes(6) |

## Consumer Control — Pass-through (BLE Report ID 1 → USB Report ID 1)

These usages work natively on macOS/Windows and are forwarded as-is:

| BLE Usage | Button | USB Usage |
|-----------|--------|-----------|
| 0x00E2 | Mute | 0x00E2 (Mute) |
| 0x0221 | Search/Mic | 0x0221 (AC Search) |

## Consumer Control — Remapped to Keyboard (BLE Report ID 1 → USB Report ID 2)

These Consumer usages are ignored by macOS/Windows, so they are translated
to standard keyboard keycodes:

| BLE Usage | Button | USB Keycode | Key |
|-----------|--------|-------------|-----|
| 0x0042 | D-pad Up | 0x52 | Arrow Up |
| 0x0043 | D-pad Down | 0x51 | Arrow Down |
| 0x0044 | D-pad Left | 0x50 | Arrow Left |
| 0x0045 | D-pad Right | 0x4F | Arrow Right |
| 0x0041 | Enter/OK | 0x28 | Enter |
| 0x0224 | Back | 0x29 | Escape |
| 0x0223 | Home | 0x05 | B (black screen in PowerPoint) |
| 0x009C | Channel + | 0x4B | Page Up |
| 0x009D | Channel - | 0x4E | Page Down |
| 0x0030 | Power | 0x68 | F13 |
| 0x019C | TV Source | 0x69 | F14 |
| 0x01BB | User | 0x6A | F15 |
| 0x009F | Settings/Gear | 0x6B | F16 |
| 0x008D | TV Guide | 0x3E | F5 |
| 0x0077 | App 1 | 0x6D | F18 |
| 0x0078 | App 2 | 0x6E | F19 |
| 0x0079 | App 3 | 0x6F | F20 |
| 0x007A | App 4 | 0x70 | F21 |

## Keyboard Report — Remapped to Consumer (BLE Report ID 0x0B → USB Report ID 1)

Volume keys arrive on BLE Report ID 11 (0x0B) as non-standard keyboard
keycodes (0x80/0x81). These don't work reliably as USB keyboard keycodes,
so they are remapped to Consumer Control usages which work universally:

| BLE Keycode | Button | USB Usage | Consumer Key |
|-------------|--------|-----------|--------------|
| 0x80 | Volume Up | 0x00E9 | Volume Up |
| 0x81 | Volume Down | 0x00EA | Volume Down |

## Dropped BLE Reports

These BLE report IDs are silently dropped (never forwarded to USB):

| BLE Report ID | Type | Reason |
|---------------|------|--------|
| 0x10 | Vendor (0xFF81) | Causes OS misclassification |
| 0x0C | Vendor (0xFF82) | Causes OS misclassification |
| 0x03 | Mouse | Not used by remote |
| 0x0D | System Control | Not used by remote |
