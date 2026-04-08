# ONN Remote Integration Findings

## Status
Pairing works. Reconnect after powercycle **not yet tested** (latest fix compiled but not flashed).

## Button Mapping

All buttons use **Report ID=1** (Consumer Control, 16-bit LE) except volume.

| Bytes (LE) | HID Usage | Button |
|---|---|---|
| `43 00` | 0x0043 | Select/OK |
| `44 00` | 0x0044 | Up |
| `45 00` | 0x0045 | Down |
| `42 00` | 0x0042 | Right |
| `41 00` | 0x0041 | Left |
| `23 02` | 0x0223 | Home |
| `24 02` | 0x0224 | Back |
| `21 02` | 0x0221 | Search/Mic |
| `E2 00` | 0x00E2 | Mute |
| `9C 00` | 0x009C | Channel Up |
| `9D 00` | 0x009D | Channel Down |
| `78 00` | 0x0078 | Rewind |
| `77 00` | 0x0077 | Fast Forward |
| `79 00` | 0x0079 | Play |
| `7A 00` | 0x007A | Pause |
| `9F 00` | 0x009F | Info/Guide |
| `30 00` | 0x0030 | Power |
| `8D 00` | 0x008D | Settings/Gear |
| `9C 01` | 0x019C | TV Input |
| `BB 01` | 0x01BB | Bookmark/Star |

Volume uses **Report ID=11** (Keyboard):
| Byte | Button |
|---|---|
| `80` | Vol Up |
| `81` | Vol Down |

## BLE Privacy / Reconnect Issue

- ONN remote uses BLE privacy (RPA - Resolvable Private Address)
- Advertises as MAC `00:00:00:00:00:00` when identity not resolved
- After bonding, NimBLE resolves to real MAC `CA:47:39:34:AC:97`
- After ESP powercycle, bonds persist in NVS but `isBonded(00:00:00:00:00:00)` returns false
- Cannot `createClient(00:00:00:00:00:00)` — connection fails immediately
- **Latest fix:** use `createClient()` (no address) + `connect(advertisedDevice, ...)` so NimBLE resolves IRK from advertisement data

## Crash Fixed

Guru Meditation (LoadProhibited) after HID Report Map dump — added `pClient->isConnected()` checks before battery service access and `deleteBond`.

## Next Steps

1. Test reconnect with `advertisedDevice`-based connect fix
2. Remove debug prints and `CONFIG_DEBUG_MODE`, build production firmware
3. Consider flashing app at `0x10000` instead of merged binary at `0x0` to preserve NVS/bonds during development
