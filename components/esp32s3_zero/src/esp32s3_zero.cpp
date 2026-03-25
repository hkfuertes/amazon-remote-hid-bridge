#include "esp32s3_zero.hpp"

using namespace espp;

bool Esp32s3Zero::initialize_led() {
  logger_.info("Initializing NeoPixel on GPIO {}", static_cast<int>(led_data_io));
  led_ = std::make_shared<Neopixel>(Neopixel::Config{
      .data_gpio = led_data_io,
      .power_gpio = led_power_io,
      .num_leds = num_leds_,
      .log_level = espp::Logger::Verbosity::WARN,
  });
  return true;
}

bool Esp32s3Zero::led(const Rgb &rgb) {
  if (!led_) return false;
  led_->set_all(rgb);
  led_->show();
  return true;
}

bool Esp32s3Zero::led(const Hsv &hsv) {
  if (!led_) return false;
  led_->set_all(hsv);
  led_->show();
  return true;
}

bool Esp32s3Zero::initialize_button(const button_callback_t &callback) {
  logger_.info("Initializing boot button on GPIO {}", static_cast<int>(button_io));
  // Configure the boot button pin
  gpio_config_t btn_cfg = {};
  btn_cfg.pin_bit_mask = 1ULL << button_io;
  btn_cfg.mode = GPIO_MODE_INPUT;
  btn_cfg.pull_up_en = GPIO_PULLUP_ENABLE;
  btn_cfg.pull_down_en = GPIO_PULLDOWN_DISABLE;
  btn_cfg.intr_type = GPIO_INTR_ANYEDGE;
  gpio_config(&btn_cfg);

  if (callback) {
    interrupts_.add_interrupt(espp::Interrupt::PinConfig{
        .gpio_num = button_io,
        .callback =
            [this, callback](const auto &event) {
              bool active = !gpio_get_level(button_io); // active low
              callback(espp::Interrupt::Event{.active = active});
            },
        .active_level = espp::Interrupt::ActiveLevel::LOW,
        .interrupt_type = espp::Interrupt::Type::ANY_EDGE,
        .pullup_enabled = true,
    });
  }
  return true;
}

bool Esp32s3Zero::button_state() const {
  return !gpio_get_level(button_io); // active low
}
