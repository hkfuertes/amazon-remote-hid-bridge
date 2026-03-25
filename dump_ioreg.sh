#!/bin/bash
# Buscar el descriptor HID del mando Amazon en IOKit
echo "=== Buscando dispositivos BLE HID en IOKit ==="

# Método 1: IOHIDDevice
echo -e "\n--- IOHIDDevice ---"
ioreg -lrx -n IOHIDDevice 2>/dev/null | grep -iE "(Product|Manufacturer|ReportDescriptor|Transport)" | head -40

# Método 2: Bluetooth HID
echo -e "\n--- Bluetooth HID ---"
ioreg -lrx -p IOService 2>/dev/null | grep -iB5 -A5 "ReportDescriptor" | head -60

# Método 3: Buscar "AR" o "Amazon" en todo el IORegistry
echo -e "\n--- Buscando 'AR' en IORegistry ---"
ioreg -lrx 2>/dev/null | grep -iB3 -A10 '"AR"' | head -60

# Método 4: system_profiler Bluetooth
echo -e "\n--- system_profiler Bluetooth (dispositivos conectados) ---"
system_profiler SPBluetoothDataType 2>/dev/null | grep -A15 "AR:" | head -20
system_profiler SPBluetoothDataType 2>/dev/null | grep -A15 "Amazon" | head -20

# Método 5: hidutil list
echo -e "\n--- hidutil list ---"
hidutil list 2>/dev/null | grep -iE "(AR|Amazon|1949)" | head -10
