FROM espressif/idf:v5.5.1

WORKDIR /project

# Copy project files
COPY . .

# Delete any previous build artifacts and sdkconfig
RUN rm -rf build sdkconfig managed_components dependencies.lock

# Build the project
RUN bash -c "source /opt/esp/idf/export.sh && \
    idf.py set-target esp32s3 && \
    idf.py build"

# Generate merged binary for WebSerial flashing (single file at offset 0x0)
RUN bash -c "source /opt/esp/idf/export.sh && \
    cd build && \
    esptool.py --chip esp32s3 merge_bin \
      -o atv-hid-bridge.bin \
      --flash_mode dio \
      --flash_freq 80m \
      --flash_size 4MB \
      0x0 bootloader/bootloader.bin \
      0x8000 partition_table/partition-table.bin \
      0x9000 nvs_flash/nvs_partition_table.bin \
      0x10000 esp-usb-ble-hid.bin \
      2>/dev/null || \
    esptool.py --chip esp32s3 merge_bin \
      -o atv-hid-bridge.bin \
      --flash_mode dio \
      --flash_freq 80m \
      --flash_size 4MB \
      0x0 bootloader/bootloader.bin \
      0x8000 partition_table/partition-table.bin \
      0x10000 esp-usb-ble-hid.bin"

# The merged binary will be at /project/build/atv-hid-bridge.bin
CMD ["echo", "Build complete. Extract with: docker cp <container>:/project/build/atv-hid-bridge.bin ."]
