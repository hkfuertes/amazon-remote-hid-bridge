#include "usb.hpp"
#include "amazon_remote_desc.h"

static espp::Logger logger({.tag = "USB"});

/************* TinyUSB descriptors ****************/

//--------------------------------------------------------------------+
// Device Descriptors
//--------------------------------------------------------------------+

#define TUSB_DESC_TOTAL_LEN (TUD_CONFIG_DESC_LEN + CFG_TUD_HID * TUD_HID_INOUT_DESC_LEN)
static_assert(CFG_TUD_HID >= 1, "CFG_TUD_HID must be at least 1");

// Use the Amazon Remote HID descriptor directly
static const uint8_t* hid_report_descriptor     = amazon_remote_hid_desc;
static const uint16_t hid_report_descriptor_len = amazon_remote_hid_desc_len;
static uint8_t usb_hid_input_report[CFG_TUD_HID_EP_BUFSIZE];
static size_t usb_hid_input_report_len = 0;

void device_event_handler(tinyusb_event_t *event, void *arg);

static tusb_desc_device_t desc_device = {.bLength = sizeof(tusb_desc_device_t),
                                         .bDescriptorType = TUSB_DESC_DEVICE,
                                         .bcdUSB = 0x0200,
                                         .bDeviceClass = 0x00,
                                         .bDeviceSubClass = 0x00,
                                         .bDeviceProtocol = 0x00,

                                         .bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,

                                         .idVendor = 0x303A,  // Espressif
                                         .idProduct = 0x8105, // ESP HID Bridge
                                         .bcdDevice = 0x0100,

                                         // Index of manufacturer description string
                                         .iManufacturer = 0x01,
                                         // Index of product description string
                                         .iProduct = 0x02,
                                         // Index of serial number description string
                                         .iSerialNumber = 0x03,
                                         // Number of configurations
                                         .bNumConfigurations = 0x01};

static const char *hid_string_descriptor[5] = {
    // array of pointer to string descriptors
    (char[]){0x09, 0x04}, // 0: is supported language is English (0x0409)
    "Amazon",             // 1: Manufacturer
    "Fire TV Remote",     // 2: Product
    "00000001",           // 3: Serials
    "USB HID Interface",  // 4: HID
};

// update the configuration descriptor with the new report descriptor size
static uint8_t hid_configuration_descriptor[] = {
    // Configuration number, interface count, string index, total length, attribute, power in mA
    TUD_CONFIG_DESCRIPTOR(1, 1, 0, TUSB_DESC_TOTAL_LEN, 0x00, 100),

    // Interface number, string index, boot protocol, report descriptor len, EP In address, size &
    // polling interval
    TUD_HID_INOUT_DESCRIPTOR(0, 4, HID_ITF_PROTOCOL_NONE, amazon_remote_hid_desc_len, 0x01, 0x81,
                             CFG_TUD_HID_EP_BUFSIZE, 1),
};

void start_usb_hid() {
  tinyusb_config_t tusb_cfg = TINYUSB_DEFAULT_CONFIG(device_event_handler);
  tusb_cfg.task = TINYUSB_TASK_CUSTOM(4096 /*size */, 4 /* priority */,
                                      0 /* affinity: 0 - CPU0, 1 - CPU1 ... */);
  tusb_cfg.descriptor.device = &desc_device;
  tusb_cfg.descriptor.string = hid_string_descriptor;
  tusb_cfg.descriptor.string_count =
      sizeof(hid_string_descriptor) / sizeof(hid_string_descriptor[0]);
  tusb_cfg.descriptor.full_speed_config = hid_configuration_descriptor;
  tusb_cfg.phy.skip_setup = false; // was external-phy = false
  tusb_cfg.phy.self_powered = false;
  tusb_cfg.phy.vbus_monitor_io = -1;

  if (tinyusb_driver_install(&tusb_cfg) != ESP_OK) {
    logger.error("Failed to install tinyusb driver");
    return;
  }
  logger.info("USB initialization DONE");
}

void stop_usb_hid() {
  if (tinyusb_driver_uninstall() != ESP_OK) {
    logger.error("Failed to uninstall tinyusb driver");
    return;
  }
  logger.info("USB initialization DONE");
}

bool send_hid_report(uint8_t report_id, const std::vector<uint8_t> &report) {
  if (report.size() == 0 || report.size() > CFG_TUD_HID_EP_BUFSIZE) {
    return false;
  }
  // copy the report data into the usb_hid_input_report buffer
  std::memcpy(usb_hid_input_report, report.data(), report.size());
  usb_hid_input_report_len = report.size();
  // now try to send it
  return tud_hid_report(report_id, usb_hid_input_report, usb_hid_input_report_len);
}

/********* TinyUSB HID callbacks ***************/

// cppcheck-suppress constParameterCallback
void device_event_handler(tinyusb_event_t *event, void *arg) {
  switch (event->id) {
  case TINYUSB_EVENT_ATTACHED: {
    logger.info("USB Mounted");
  } break;
  case TINYUSB_EVENT_DETACHED: {
    logger.info("USB Unmounted");
  } break;
  default:
    break;
  }
}

// Invoked when received GET HID REPORT DESCRIPTOR request
// Application return pointer to descriptor, whose contents must exist long enough for transfer to
// complete
extern "C" uint8_t const *tud_hid_descriptor_report_cb(uint8_t instance) {
  return amazon_remote_hid_desc;
}

// Invoked when received GET_REPORT control request
// Application must fill buffer report's content and return its length.
// Return zero will cause the stack to STALL request
extern "C" uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id,
                                          hid_report_type_t report_type, uint8_t *buffer,
                                          uint16_t reqlen) {
  // copy the report data into the buffer
  // NOTE: we're ignoring the report_id here
  switch (report_type) {
  case HID_REPORT_TYPE_INVALID:
    return 0;
  case HID_REPORT_TYPE_INPUT:
    std::memcpy(buffer, usb_hid_input_report, usb_hid_input_report_len);
    return usb_hid_input_report_len;
  case HID_REPORT_TYPE_OUTPUT:
    return 0;
  case HID_REPORT_TYPE_FEATURE:
    return 0;
  }
  return 0;
}

// Invoked when received SET_REPORT control request or
// received data on OUT endpoint ( Report ID = 0, Type = 0 )
extern "C" void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id,
                                      hid_report_type_t report_type, uint8_t const *buffer,
                                      uint16_t bufsize) {
  // Amazon remote pass-through: no processing needed on SET_REPORT
}

// Invoked when sent REPORT successfully to host
// Application can use this to send the next report
// Note: For composite reports, report[0] is report ID
extern "C" void tud_hid_report_complete_cb(uint8_t instance, uint8_t const *reprot, uint16_t len) {
  // TODO: debug this
}
