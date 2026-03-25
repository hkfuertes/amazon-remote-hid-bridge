#!/bin/bash
set -e

IMAGE_NAME="amazon-remote-hid-bridge"
CONTAINER_NAME="amazon-remote-hid-bridge-build"
OUTPUT_BIN="amazon-remote-hid-bridge.bin"

echo "=== Building firmware with Docker ==="
docker build -t "$IMAGE_NAME" .

echo "=== Extracting merged binary ==="
docker rm -f "$CONTAINER_NAME" 2>/dev/null || true
docker create --name "$CONTAINER_NAME" "$IMAGE_NAME"
docker cp "$CONTAINER_NAME:/project/build/$OUTPUT_BIN" "./$OUTPUT_BIN"
docker rm "$CONTAINER_NAME"

echo ""
echo "=== Done! ==="
echo "Flash with WebSerial (e.g. https://web.esphome.io):"
echo "  - Connect ESP32-S3-Zero via USB"
echo "  - Hold BOOT button, press RESET, release BOOT"
echo "  - Select the serial port"
echo "  - Flash '$OUTPUT_BIN' at offset 0x0"
echo ""
echo "Binary: ./$OUTPUT_BIN"
